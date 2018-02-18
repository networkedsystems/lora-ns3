/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 CTTC
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
#include "ns3/lora-phy-gw.h"
#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/channel.h"
#include "ns3/trace-source-accessor.h"
#include "lora-mac-header.h"
#include "lora-mac-command.h"
#include "lora-tsch-net-device-gw.h"
#include "lora-net-device-gw.h"
#include "lora-tsch-net-device.h"
#include "lora-net-device.h"
#include <ns3/mac32-address.h>
#include "commands/link-adr-req.h"
namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaTschNetDeviceGw");

	NS_OBJECT_ENSURE_REGISTERED (LoRaTschNetDeviceGw);

	TypeId
		LoRaTschNetDeviceGw::GetTypeId (void)
		{
	static TypeId tid = TypeId ("ns3::LoRaTschNetDeviceGw")
				.SetParent<LoRaTschNetDevice> ()
				.AddConstructor<LoRaTschNetDeviceGw> ()
				.AddAttribute ("Offset", "Beacon offset",
						UintegerValue (0),
						MakeUintegerAccessor (&LoRaTschNetDeviceGw::m_offset),//::SetOffset,
				MakeUintegerChecker<uint32_t> ())
						//									&LoRaTschNetDevice::GetOffset),
					//.AddTraceSource ("MacTxGw",
					//                 "Trace source indicating a packet has arrived "
					//                 "for transmission by this device",
					//                 MakeTraceSourceAccessor (&LoRaTschNetDeviceGw::m_macTxTrace),
					//                 "ns3::Packet::TracedCallback")
					//.AddTraceSource ("MacTxDropGw",
					//                 "Trace source indicating a packet has been dropped "
					//                 "by the device before transmission",
					//                 MakeTraceSourceAccessor (&LoRaTschNetDeviceGw::m_macTxDropTrace),
					//                 "ns3::Packet::TracedCallback")
					//.AddTraceSource ("MacPromiscRxGw",
					//                 "A packet has been received by this device, has been "
					//                 "passed up from the physical layer "
					//                 "and is being forwarded up the local protocol stack.  "
					//                 "This is a promiscuous trace,",
					//                 MakeTraceSourceAccessor (&LoRaTschNetDeviceGw::m_macPromiscRxTrace),
					//                 "ns3::Packet::TracedCallback")
					;
			return tid;
		}

	LoRaTschNetDeviceGw::LoRaTschNetDeviceGw ()
		: LoRaNetDeviceGw()
	{
		NS_LOG_FUNCTION (this);
		m_interBeacon = Seconds(60);
	}

	LoRaTschNetDeviceGw::~LoRaTschNetDeviceGw ()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaTschNetDeviceGw::DoInitialize (void)
		{
			m_beacon = Simulator::Schedule(MilliSeconds(10),&LoRaTschNetDeviceGw::SendBeacon,this);
			LoRaNetDeviceGw::DoInitialize();
			memset(&m_receptionsPerChannel,0,16);
			memset(&m_rssiValues,180,16);
			m_rssiValues[0] = 80;
			m_rssiValues[1] = 110;
			m_rssiValues[2] = 140;
			m_availableChannels = 3;
		}

	void
		LoRaTschNetDeviceGw::DoDispose ()
		{
			NS_LOG_FUNCTION (this);
			LoRaNetDevice::DoDispose ();
		}

	void
		LoRaTschNetDeviceGw::DoCheckAckSend (Ptr<const Packet> packet, uint32_t frequency, uint8_t datarate, uint8_t powerIndex)
		{
			NS_LOG_FUNCTION (this);
			if (packet!=0)
			{
				if ((this->GetPhy()->GetReceptions()>0 || m_phy->IsTransmitting()) && (Simulator::GetDelayLeft(m_beacon) <= m_phy->GetTimeOfPacket(packet->GetSize(),12-datarate)))
				{
					Simulator::Schedule(Seconds(1),&LoRaTschNetDeviceGw::StartTransmission,this,packet->Copy (),frequency,datarate,powerIndex);
				}
				else{
					Simulator::ScheduleNow(&LoRaTschNetDeviceGw::StartTransmission,this,packet->Copy (),frequency,datarate,powerIndex);
				}
			}
		}

	//void
	//LoRaTschNetDeviceGw::NotifyReceptionEndOk (Ptr<Packet> packet, uint32_t bandwidth, uint8_t spreading, uint32_t frequency, double rssi)
	//{
	//  NS_LOG_FUNCTION (this << packet);
	//  LoRaMacHeader header;
	//  packet->PeekHeader (header);
	//  NS_LOG_LOGIC ("packet " << header.GetAddr () << " --> basestation (here: " << this->GetAddress() << ")");

	//  PacketType packetType;
	//  // We ignore broadasts
	//  if (header.GetAddr ().IsBroadcast ())
	//    {
	//      packetType = PACKET_OTHERHOST;
	//    }
	//  if (header.GetAddr ().IsGroup ())
	//    {
	//      packetType = PACKET_MULTICAST;
	//    }
	//  else
	//    {
	//      packetType = PACKET_HOST;
	//    }

	//  NS_LOG_LOGIC ("packet type = " << packetType);

	//  if (!m_rssi.IsNull() && (header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_UNCONFIRMED_DATA_UP || header.GetType() == LoRaMacHeader::LoRaMacType::LORA_MAC_CONFIRMED_DATA_UP))
	//    {
	//      std::list<LoRaMacCommand*> commands = header.GetCommandList ();
	//      for (std::list<LoRaMacCommand*>::iterator it = commands.begin(); it!=commands.end();++it)
	//        {
	//          (*it)->Execute(this,header.GetAddr());
	//        }
	//      m_rssi(rssi,header.GetAddr ());
	//    }
	//  //std::cout << "0.025," << frequency << "," << (uint32_t)spreading << "," << rssi << "," << 10 <<std::endl;
	//  if (!m_promiscRxCallback.IsNull ())
	//    {
	//      m_promiscRxCallback (this, packet->Copy (), 0, header.GetAddr (), header.GetAddr (), packetType);
	//    }
	//  if (packetType != PACKET_OTHERHOST)
	//    {
	//      m_macPromiscRxTrace(packet);
	//      m_receptionsPerChannel[GetChannelNbFromFrequency(frequency)]++;
	//    }
	//  if (!m_rxCallback.IsNull ())
	//    {
	//      m_rxCallback (this, packet->Copy(), header.GetPort (), header.GetAddr ());
	//      m_pendingAcks.push_back(std::make_tuple(header.GetAddr (), Simulator::Now()+Seconds(1), Create<Packet>(0)));
	//      Simulator::Schedule(Seconds(1.1),&LoRaTschNetDeviceGw::PruneAck,this,header.GetAddr());
	//      // \TODO Remove as we do not need in anymore. Move to network server.
	//      //Simulator::Schedule(Seconds(m_delay),&LoRaTschNetDeviceGw::CheckAckSend, this, ack,frequency, dr, 2);
	//    }
	//}

	void
		LoRaTschNetDeviceGw::SendBeacon()
		{
			NS_LOG_FUNCTION(this);
			Ptr<Packet> beacon = Create<Packet> (0);
			LoRaMacHeader beaconHeader;
			beaconHeader.SetType(LoRaMacHeader::LORA_MAC_BEACON);
			beaconHeader.SetFrameVer (1);
			beaconHeader.SetAddr (Mac32Address("255:255:255:254").GetBroadcast());
			uint8_t m_sortedReceptions [m_availableChannels];
			uint32_t minutes = Simulator::Now().GetMinutes();
			for (uint8_t i = 0;i<m_availableChannels;i++)
			{
				m_sortedReceptions[i] = m_receptionsPerChannel[(minutes-1+i)%m_availableChannels];
				m_sortedReceptions[i] = m_receptionsPerChannel[i];
			}
			for (uint8_t i = 1;i<m_availableChannels;i++)
			{
				int difference  = (((int)m_sortedReceptions[i]-(int)m_sortedReceptions[i-1])+1);
				if (difference > 0)
					m_rssiValues[i] += std::log2(std::abs(difference));
				else if(difference < 0)
					m_rssiValues[i] -= std::log2(std::abs(difference));
			}
			for (uint8_t i = 1;i<m_availableChannels-1;i++)
			{
				if(m_rssiValues[i]>m_rssiValues[i+1])
				{
					uint8_t temp = m_rssiValues[i];
					m_rssiValues[i]=m_rssiValues[i+1];
					m_rssiValues[i+1]=temp;
				}
			}
			for (uint8_t i = 0;i<m_availableChannels;i++)
			{
				m_receptionsPerChannel[i]*=0.5;
			}
			m_rssiValues[1]=169;
			m_rssiValues[2]=176;
			m_rssiValues[0]=m_rssiValues[1]-15;
			//beaconHeader.AddChannel(m_rssiValues[0],0x3F);// [minutes%m_availableChannels],0x3f);
			//beaconHeader.AddChannel(m_rssiValues[1],0x3F);// [minutes%m_availableChannels],0x3f);
			//beaconHeader.AddChannel(m_rssiValues[2],0x3F);// [minutes%m_availableChannels],0x3f);
			for(uint8_t i = 0; i<m_availableChannels;i++)
			{
				//if ((minutes+i)%m_availableChannels>0 && (i == 1 || i == 2))
				//	beaconHeader.AddChannel(m_rssiValues[(minutes+i)%m_availableChannels],0x01);
				//else
					//beaconHeader.AddChannel(m_rssiValues[(minutes+i)%m_availableChannels],0x3f);
					beaconHeader.AddChannel(m_rssiValues[(i)%m_availableChannels],0x3f);
			}
			beacon->AddHeader(beaconHeader);
			//std::cout << " I am sending beacon with offset " << m_offset << " " << (uint32_t)m_availableChannels <<" I was listening to freq: " << frequencies[(m_offset + (uint32_t)Simulator::Now().GetMinutes())%3]<<std::endl;
			StartTransmission(beacon,frequencies[(minutes+m_offset)%m_availableChannels],3,1);
			m_beacon = Simulator::Schedule(m_interBeacon,&LoRaTschNetDeviceGw::SendBeacon,this);
		}

	void
		LoRaTschNetDeviceGw::SetOffset (uint32_t offset)
		{
			m_offset = offset;
		}

	uint32_t 
		LoRaTschNetDeviceGw::GetOffset ()
		{
			return m_offset;
		}

} // namespace ns3
