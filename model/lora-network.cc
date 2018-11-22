/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 KU Leuven
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/net-device.h"
#include "lora-network.h"
#include "lora-mac-header.h"
#include "gw-trailer.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/channel.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac32-address.h"
#include "ns3/address-utils.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/socket-factory.h"
#include "lora-network-trailer.h"

#include <iostream>
#include <list>
#include <string>
#include <cctype>
#include <map>

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaNetwork");

	NS_OBJECT_ENSURE_REGISTERED (LoRaNetwork);

	TypeId 
		LoRaNetwork::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaNetwork")
				.AddConstructor<LoRaNetwork> ()
				.SetParent<Application> ()
				.SetGroupName("lora")
				.AddTraceSource ("NetRx",
						"network has received a new packet",
						MakeTraceSourceAccessor (&LoRaNetwork::m_netRxTrace),
						"ns3::Packet::TracedCallback")
				.AddTraceSource ("NetPromiscRx",
						"network has received a packet",
						MakeTraceSourceAccessor (&LoRaNetwork::m_netPromiscRxTrace),
						"ns3::Packet::TracedCallback")
				.AddAttribute ("Port",
						"The port to listen on server",
						UintegerValue(100),
						MakeUintegerAccessor (&LoRaNetwork::m_port),
						MakeUintegerChecker<uint16_t> ())
				;
			return tid;
		}

	LoRaNetwork::LoRaNetwork ()
	{
		NS_LOG_FUNCTION (this);
	}

	LoRaNetwork::~LoRaNetwork ()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaNetwork::DoDispose ()
		{
			NS_LOG_FUNCTION (this);
			m_stats.clear();
			//::DoDispose();
		}

	void LoRaNetwork::StartApplication(void)
	{
		NS_LOG_FUNCTION (this);

		if (m_socket == 0)
		{
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			m_socket = Socket::CreateSocket (GetNode (), tid);
			InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
			m_socket->Bind (local);
			if (addressUtils::IsMulticast (local))
			{
				Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
				if (udpSocket)
				{
					// equivalent to setsockopt (MCAST_JOIN_GROUP)
					udpSocket->MulticastJoinGroup (0, local);
				}
				else
				{
					NS_FATAL_ERROR ("Error: Failed to join multicast group");
				}
			}
		} 
		m_socket->SetRecvCallback (MakeCallback (&LoRaNetwork::HandleRead, this));
	}

	void LoRaNetwork::StopApplication (void)
	{
		NS_LOG_FUNCTION (this);
		m_socket->Close();
	}

	bool
		LoRaNetwork::MessageReceived (Ptr<const Packet> packet, const Address &from)
		{
			NS_LOG_FUNCTION (this << packet << from);
			LoRaMacHeader header;
			packet->PeekHeader(header);
			Address address = header.GetAddr();
			NS_LOG_FUNCTION (this << address );
			uint16_t seqnum = header.GetFrmCounter();
			GwTrailer trailer;
			packet->Copy()->PeekTrailer (trailer);
			Simulator::ScheduleNow(&LoRaNetwork::m_netPromiscRxTrace,this,packet->Copy());
			if (IsWhiteListed(address))
			{
				if (m_stats.find(address) != m_stats.end())
				{
					NS_LOG_LOGIC("There has been a message that is just transmitted");
					// Send message to gateway that there is no need to transmit anything
					// we use the ACK field for downlink message to tell if a gateway needs to send something

					//LoRaMacHeader ackHeader;
					//Ptr<Packet> ack = Create<Packet> (0);
					//ackHeader.SetAddr(header.GetAddr ());
					//ackHeader.SetNoAck ();
					m_stats[address].gwCount++;
					if (m_stats[address].maxRssi < trailer.GetRssi ())
					{
						m_stats[address].maxRssi = trailer.GetRssi ();
						m_stats[address].strongestGateway = from;
					}
					return false;
				}
				PacketStats stats;
				stats.maxRssi = trailer.GetRssi();
				stats.gwCount = 1;
				stats.strongestGateway = from;
				m_stats[address] = stats;
				if(m_latest[address]!=seqnum)
				{
					m_latest[address] = seqnum;
					Simulator::ScheduleNow(&LoRaNetwork::m_netRxTrace,this,packet);
					m_packetToTransmit[header.GetAddr()] = 0;
				}
				if ((header.IsAdrAck () && header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_UNCONFIRMED_DATA_UP) || header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_CONFIRMED_DATA_UP)
				{
					LoRaMacHeader ackHeader;
					ackHeader = LoRaMacHeader(LoRaMacHeader::LoRaMacType::LORA_MAC_UNCONFIRMED_DATA_DOWN,header.GetFrmCounter());
					Ptr<Packet> ack = Create<Packet> (0);
					ackHeader.SetAddr(header.GetAddr ());
					ackHeader.SetAck ();
					ack->AddHeader (ackHeader);
					DeviceRxSettings theseSettings = m_settings[header.GetAddr()];
					LoRaNetworkTrailer trailer = LoRaNetworkTrailer(theseSettings.delay,theseSettings.dr1Offset,theseSettings.dr2,theseSettings.frequency);
					ack->AddTrailer (trailer);
					Address address = header.GetAddr();
					m_packetToTransmit[address] = ack;
				}
				NS_LOG_LOGIC ("Scheduling ACK or data");
				Simulator::Schedule(Seconds(0.5),&LoRaNetwork::SendAck,this,address);
				return true;
			}
			return false;
		}


	bool 
		LoRaNetwork::IsWhiteListed (const Address& address)
		{
			for (auto &it : m_whiteList )
			{
				if (it==address)
				{
					return true;
				}
			}
			return false;
		}

	void
		LoRaNetwork::WhiteListDevice (const Address& address)
		{
			m_whiteList.push_back(address);
			//m_whitelistCallback(address);
			m_latest[address] = 255;
			DeviceRxSettings settings;
			settings.delay = 1;
			settings.dr1Offset = 0;
			settings.dr2 = 0;
			settings.frequency = 8695250;
			m_settings[address]=settings;
		}

	void 
		LoRaNetwork::SendAck (const Address& sensor)
		{
			NS_LOG_FUNCTION (this << sensor);
			if (m_packetToTransmit[sensor] != 0)
			{
				LoRaMacHeader header; 
				m_packetToTransmit[sensor]->PeekHeader(header);
				m_socket->SendTo (m_packetToTransmit[sensor],0,m_stats[header.GetAddr()].strongestGateway);
				m_packetToTransmit[sensor] = 0;
			}
			else 
			{
				NS_LOG_DEBUG("This is empty");
			}
			m_stats.erase(sensor);
		}

	void
		LoRaNetwork::HandleRead (Ptr<Socket> socket)
		{
			NS_LOG_FUNCTION (this << socket);

			Ptr<Packet> packet;
			Address from;
			while ((packet = socket->RecvFrom (from)))
			{
				if(packet->GetSize () > 0)
				{
					packet->RemoveAllPacketTags ();
					packet->RemoveAllByteTags ();
					MessageReceived(packet,from);
				}
			}
		}

	bool
		LoRaNetwork::Send (Ptr<const Packet> packet)
		{
			NS_LOG_FUNCTION(this << packet);
			Ptr<Packet> copy = packet->Copy();
			LoRaMacHeader mac;
			copy->PeekHeader (mac);
			if (m_packetToTransmit [mac.GetAddr()]==0)
			{
				NS_LOG_LOGIC("Queue for this device was empty.");
				// Add trailer such that GW knows what to do with it.
				DeviceRxSettings theseSettings = m_settings[mac.GetAddr()];
				LoRaNetworkTrailer trailer = LoRaNetworkTrailer(theseSettings.delay,theseSettings.dr1Offset,theseSettings.dr2,theseSettings.frequency);
				copy->AddTrailer (trailer);
				m_packetToTransmit [mac.GetAddr()] = copy;
			}
			else
			{
				NS_LOG_DEBUG("Merging packets");
				// Check if at least one of them does not contain data, otherwise drop the latter.
				LoRaMacHeader mac2;
				m_packetToTransmit[mac.GetAddr()]->PeekHeader(mac2);
				NS_LOG_DEBUG(mac2.GetSerializedSize() << " " << copy->GetSerializedSize () << " " << m_packetToTransmit[mac.GetAddr ()]->GetSerializedSize()<<  " " << mac.GetSerializedSize ());
				if (mac2.GetSerializedSize () == m_packetToTransmit[mac.GetAddr()]->GetSize () || mac.GetSerializedSize()==copy->GetSize())
				{
					// keep the packet with data
					if (packet->GetSize () - mac.GetSerializedSize () > 0)
					{
						copy->RemoveHeader(mac);
						mac.Merge(mac2);
						copy->AddHeader(mac);
						m_packetToTransmit[mac.GetAddr()] = 0;
						m_packetToTransmit[mac.GetAddr()] = copy;
					}
					else
					{
						m_packetToTransmit[mac.GetAddr()]->RemoveHeader (mac2);
						mac2.Merge (mac);
						m_packetToTransmit[mac.GetAddr()]->AddHeader (mac2);
					}
					return true;

				}
			}
			return false;
		}

	void LoRaNetwork::SetDelayOfDevice (const Address& address, uint8_t delay)
	{
		m_settings[address].delay = delay;
	}
	void LoRaNetwork::SetSettingsOfDevice (const Address& address, uint8_t offset, uint8_t dr, uint32_t freq)
	{
		m_settings[address].dr1Offset = offset;
		m_settings[address].dr2 = dr;
		m_settings[address].frequency = freq;
	}

	uint8_t LoRaNetwork::GetCount (const Address& address)
	{
		return m_stats[address].gwCount;
	}

	uint8_t LoRaNetwork::GetMargin (const Address& address)
	{
		double temp =  10*std::log10(m_stats[address].maxRssi)+160;
		if (temp > 254) 
			return 254;
		else
			return (uint8_t) temp;
	}

} // namespace ns3
