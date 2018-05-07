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
#include "ns3/lora-phy.h"
#include "ns3/lora-gw-phy.h"
#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/channel.h"
#include "ns3/trace-source-accessor.h"
#include "lora-mac-header.h"
#include "lora-mac-command.h"
#include "lora-gw-net-device.h"
#include "lora-net-device.h"
#include "gw-trailer.h"
#include "lora-network-trailer.h"
#include "commands/link-adr-req.h"
namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaGwNetDevice");
	NS_OBJECT_ENSURE_REGISTERED (LoRaGwNetDevice);

	TypeId
		LoRaGwNetDevice::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaGwNetDevice")
				.SetParent<LoRaNetDevice> ()
				.AddConstructor<LoRaGwNetDevice> ()
				.AddTraceSource ("MacTxGw",
						"Trace source indicating a packet has arrived "
						"for transmission by this device",
						MakeTraceSourceAccessor (&LoRaGwNetDevice::m_macTxTrace),
						"ns3::Packet::TracedCallback")
				.AddTraceSource ("MacTxDropGw",
						"Trace source indicating a packet has been dropped "
						"by the device before transmission",
						MakeTraceSourceAccessor (&LoRaGwNetDevice::m_macTxDropTrace),
						"ns3::Packet::TracedCallback")
				.AddTraceSource ("MacPromiscRxGw",
						"A packet has been received by this device, has been "
						"passed up from the physical layer "
						"and is being forwarded up the local protocol stack.  "
						"This is a promiscuous trace,",
						MakeTraceSourceAccessor (&LoRaGwNetDevice::m_macPromiscRxTrace),
						"ns3::Packet::TracedCallback")
				;
			return tid;
		}

	LoRaGwNetDevice::LoRaGwNetDevice ()
		: LoRaNetDevice()
	{
		NS_LOG_FUNCTION (this);
		m_delay = 1;
	}

	void
	LoRaGwNetDevice::DoInitialize ()
	{
		LoRaNetDevice::DoInitialize ();
		SetAddress (Mac32Address ("00:00:00:01"));
	}

	LoRaGwNetDevice::~LoRaGwNetDevice ()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaGwNetDevice::DoDispose ()
		{
			NS_LOG_FUNCTION (this);
			NetDevice::DoDispose ();
		}

	Ptr<LoRaGwPhy>
		LoRaGwNetDevice::GetPhy () const
		{
			NS_LOG_FUNCTION (this);
			return DynamicCast<LoRaGwPhy>(LoRaNetDevice::GetPhy ());
		}

	bool
		LoRaGwNetDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
		{
			NS_LOG_FUNCTION (packet << src << dest << protocolNumber);
		
			// We expect a packet with a header. 
			// The MAC protocols can only be assigned by the network server.
			LoRaMacHeader header;
			packet->RemoveHeader (header);
			
			for (std::list<std::tuple<Address,Ptr<Packet>>>::iterator it = m_pendingPackets.begin(); it!=m_pendingPackets.end();++it)
			{
				// check if there is already a pending message
				if (std::get<0>(*it) == dest)
				{
					// if ACK field in message is 0, then we do not send any message;
					if (header.IsNoAck ())
					{
						m_pendingPackets.remove (*it);
						// returns true, because the message is accepted: do not transmit
						return true;
					}
					// This should be an update of the current message in the queue. The current message is generate by this gateway to schedule the message already
					// This message is newer and will act as the new message to be transmitted.
					// this should be the case, if none, that means that the network has already sent 2 messages. We have to discard the latter. 
					// hence we return false.
					if (std::get<1>(*it)->GetSize() == header.GetSerializedSize())
					{
						m_pendingPackets.remove (*it);
						break;
					}
					else
						return false;
				}
			}
			packet->AddHeader (header);
			m_pendingPackets.push_back (std::make_tuple (dest,packet));
			m_macTxTrace(packet);
			return true;
		}

	uint8_t
		LoRaGwNetDevice::GetDatarate(uint32_t bandwidth, uint8_t spreading)
		{
			return bandwidth/125000-1+12-spreading;
		}

	void
		LoRaGwNetDevice::CheckAckSend (const Address & addr, uint32_t frequency, uint8_t datarate, uint8_t powerIndex)
		{
			Ptr<Packet> packet = 0;
			// Check in the list if there is a packet for device with address addr
			for (std::list<std::tuple<Address,Ptr<Packet>>>::iterator it = m_pendingPackets.begin(); it!=m_pendingPackets.end();++it)
			{
				if (std::get<0>(*it) == addr)
					packet = std::get<1>(*it);
			}
			this->DoCheckAckSend (packet, frequency, datarate, powerIndex);
		}
	
	void
		LoRaGwNetDevice::DoCheckAckSend (Ptr<Packet> packet, uint32_t frequency, uint8_t datarate, uint8_t powerIndex)
		{
			NS_LOG_FUNCTION (this);
			if (packet != 0)
			{
				NS_LOG_DEBUG(GetPhy ()->GetReceptions() << " " << GetPhy ()->IsTransmitting());
				LoRaNetworkTrailer trailer;
				packet->RemoveTrailer(trailer);
				if (this->GetPhy ()->GetReceptions()>0 || this->GetPhy ()->IsTransmitting()||true)
				{
					Simulator::Schedule(Seconds(trailer.GetDelay()),&LoRaGwNetDevice::StartTransmission,this,packet->Copy (),trailer.GetRx2Freq(),trailer.GetRx2Dr(),powerIndex);
				}
				else{
					Simulator::Schedule(Seconds(trailer.GetDelay()-1),&LoRaGwNetDevice::StartTransmission,this,packet->Copy (),frequency,GetRxDatarate(datarate,trailer.GetRx1Offset()),powerIndex);
				}
			}
		}




	void
		LoRaGwNetDevice::NotifyTransmissionEnd (Ptr<const Packet> packet)
		{
			NS_LOG_FUNCTION (this << packet);
			LoRaMacHeader header;
			packet->PeekHeader (header);
			//if (header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_UNCONFIRMED_DATA_DOWN)
			//{
				RemoveFromPending (header.GetAddr ());	
			//}

		}


	void
		LoRaGwNetDevice::NotifyReceptionEndError ()
		{
			NS_LOG_FUNCTION (this);
			// packet is discarded. Nothing happens
		}

	void
		LoRaGwNetDevice::NotifyReceptionEndOk (Ptr<Packet> packet, uint32_t bandwidth, uint8_t spreading, uint32_t frequency, double rssi)
		{
			NS_LOG_FUNCTION (this << packet << rssi);
			LoRaMacHeader header;
			packet->PeekHeader (header);
			NS_LOG_LOGIC ("packet " << header.GetAddr () << " --> gateway (here: " << this->GetAddress() << ")");

			PacketType packetType;
			if (header.GetAddr ().IsBroadcast ())
			{
				packetType = PACKET_BROADCAST;
			}
			else if (header.GetAddr ().IsGroup ())
			{
				packetType = PACKET_MULTICAST;
			}
			else if (header.GetAddr () == this->GetAddress())
			{
				packetType = PACKET_HOST;
			}
			else
			{
				packetType = PACKET_OTHERHOST;
			}

			NS_LOG_LOGIC ("packet type = " << packetType);

			//if (!m_rssi.IsNull() && (header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_UNCONFIRMED_DATA_UP || header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_CONFIRMED_DATA_UP))
			//{
			//	m_rssi(rssi,header.GetAddr ());
			//}
			m_macPromiscRxTrace(packet->Copy());
			if (!m_promiscRxCallback.IsNull ())
			{
				m_promiscRxCallback (this, packet->Copy (), 0, header.GetAddr (), header.GetAddr (), packetType);
			}
			if (!m_rxCallback.IsNull () && (header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_UNCONFIRMED_DATA_UP || header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_CONFIRMED_DATA_UP))
			{ 
				GwTrailer trail;
				trail.SetRssi (rssi);
				trail.SetGateway (this->GetNode ()->GetId ());
				Ptr<Packet> copy = packet->Copy();
				copy->AddTrailer(trail);
				m_rxCallback (this, copy, header.GetPort (), header.GetAddr ());
				EventId ack = Simulator::Schedule(Seconds(m_delay),&LoRaGwNetDevice::CheckAckSend, this,header.GetAddr (), frequency, 12-spreading, 2);
			}
			if (header.IsAcknowledgment ())
				RemoveFromPending (header.GetAddr ());
		}

	void 
		LoRaGwNetDevice::RemoveFromPending (const Address& addr)
		{
				for (std::list<std::tuple<Address,Ptr<Packet>>>::iterator it = m_pendingPackets.begin(); it!=m_pendingPackets.end();++it)
				{
					if (std::get<0>(*it) == addr)
					{
						m_pendingPackets.remove(*it);
						break;
					}
				}
		} 

uint8_t 
LoRaGwNetDevice::GetRxDatarate (uint8_t dr, uint8_t offset)
{
	if (dr > offset)
		return dr - offset;
	else
		return 0;
}
} // namespace ns3
