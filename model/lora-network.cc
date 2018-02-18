/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
			justSend.clear();
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
			Simulator::ScheduleNow(&LoRaNetwork::m_netPromiscRxTrace,this,packet->Copy());
			if (IsWhiteListed(address))
			{
				for (auto &it : justSend)
				{
					if (address == it)
					{
						NS_LOG_LOGIC("There has been a message that is just transmitted");
						// Send message to gateway that there is no need to transmit anything
						// we use the ACK field for downlink message to tell if a gateway needs to send something

						LoRaMacHeader ackHeader;
						Ptr<Packet> ack = Create<Packet> (0);
						ackHeader.SetAddr(header.GetAddr ());
						ackHeader.SetNoAck ();
						return false;
					}
				}
				for (auto &it : m_latest)
				{
					NS_LOG_LOGIC (std::get <0> (it) << " " << address << " " << std::get <1> (it) << " "<< seqnum);
					if (address == std::get<0>(it))
					{
							if(std::get<1>(it)!=seqnum)
							{
							std::get<1>(it) = seqnum;
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
							Address address = header.GetAddr();
							m_packetToTransmit[address] = ack;
							NS_LOG_LOGIC ("Sending ACK");
							Simulator::Schedule(Seconds(0.5),&LoRaNetwork::SendAck,this,from,address);
						}
					}
				}
				justSend.push_back(address);
				Simulator::Schedule(Seconds(.5),&LoRaNetwork::RemoveMessage,this,address);
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
		LoRaNetwork::RemoveMessage (const Address& address)
		{
			for (std::vector<Address>::iterator it = justSend.begin(); it!= justSend.end(); it++)
			{
				if (address == *it)
				{
					justSend.erase(it);
					break;
				}
			} 
			//Do we need to do something here?
		}

	void
		LoRaNetwork::WhiteListDevice (const Address& address)
		{
			m_whiteList.push_back(address);
			//m_whitelistCallback(address);
			m_latest.push_back(std::make_tuple(address,255));
		}

	void 
		LoRaNetwork::SendAck (const Address& gateway, const Address& sensor)
		{
			NS_LOG_FUNCTION (this << gateway << sensor);
			if (m_packetToTransmit[sensor] != 0)
			{
				m_socket->SendTo (m_packetToTransmit[sensor],0,gateway);
				m_packetToTransmit[sensor] = 0;
			}
			else 
			{
				NS_LOG_DEBUG("This is empty");
			}
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
						mac.Merge(mac2);
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


} // namespace ns3
