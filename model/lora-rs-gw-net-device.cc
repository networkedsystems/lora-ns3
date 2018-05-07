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
#include "lora-rs-gw-net-device.h"
#include "lora-gw-net-device.h"
#include "lora-network-trailer.h"
#include "lora-rs-net-device.h"
#include "lora-net-device.h"
#include <ns3/mac32-address.h>
#include "commands/link-adr-req.h"
namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaRsGwNetDevice");

	NS_OBJECT_ENSURE_REGISTERED (LoRaRsGwNetDevice);

	TypeId
		LoRaRsGwNetDevice::GetTypeId (void)
		{
	static TypeId tid = TypeId ("ns3::LoRaRsGwNetDevice")
				.SetParent<LoRaRsNetDevice> ()
				.AddConstructor<LoRaRsGwNetDevice> ()
				.AddAttribute ("Offset", "Beacon offset",
						UintegerValue (0),
						MakeUintegerAccessor (&LoRaRsGwNetDevice::m_offset),//::SetOffset,
				MakeUintegerChecker<uint32_t> ())
						//									&LoRaRsNetDevice::GetOffset),
					//.AddTraceSource ("MacTxGw",
					//                 "Trace source indicating a packet has arrived "
					//                 "for transmission by this device",
					//                 MakeTraceSourceAccessor (&LoRaRsGwNetDevice::m_macTxTrace),
					//                 "ns3::Packet::TracedCallback")
					//.AddTraceSource ("MacTxDropGw",
					//                 "Trace source indicating a packet has been dropped "
					//                 "by the device before transmission",
					//                 MakeTraceSourceAccessor (&LoRaRsGwNetDevice::m_macTxDropTrace),
					//                 "ns3::Packet::TracedCallback")
					//.AddTraceSource ("MacPromiscRxGw",
					//                 "A packet has been received by this device, has been "
					//                 "passed up from the physical layer "
					//                 "and is being forwarded up the local protocol stack.  "
					//                 "This is a promiscuous trace,",
					//                 MakeTraceSourceAccessor (&LoRaRsGwNetDevice::m_macPromiscRxTrace),
					//                 "ns3::Packet::TracedCallback")
					;
			return tid;
		}

	LoRaRsGwNetDevice::LoRaRsGwNetDevice ()
		: LoRaGwNetDevice()
	{
		NS_LOG_FUNCTION (this);
		m_interBeacon = Seconds(60);
	}

	LoRaRsGwNetDevice::~LoRaRsGwNetDevice ()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaRsGwNetDevice::DoInitialize (void)
		{
			m_beacon = Simulator::Schedule(MilliSeconds(10),&LoRaRsGwNetDevice::SendBeacon,this);
			LoRaGwNetDevice::DoInitialize();
			memset(&m_receptionsPerChannel,0,16);
			memset(&m_rssiValues,180,16);
			m_rssiValues[0] = 80;
			m_rssiValues[1] = 110;
			m_rssiValues[2] = 140;
			m_availableChannels = 3;
		}

	void
		LoRaRsGwNetDevice::DoDispose ()
		{
			NS_LOG_FUNCTION (this);
			LoRaNetDevice::DoDispose ();
		}

	void
		LoRaRsGwNetDevice::DoCheckAckSend (Ptr<const Packet> packet, uint32_t frequency, uint8_t datarate, uint8_t powerIndex)
		{
			NS_LOG_FUNCTION (this);
			if (packet!=0)
			{
				Ptr<Packet> copy = packet->Copy();
				LoRaNetworkTrailer trailer;
				copy->RemoveTrailer(trailer);
				if ((this->GetPhy()->GetReceptions()>0 || m_phy->IsTransmitting()) && (Simulator::GetDelayLeft(m_beacon) <= m_phy->GetTimeOfPacket(packet->GetSize(),12-datarate)))
				{
					Simulator::Schedule(Seconds(trailer.GetDelay()),&LoRaRsGwNetDevice::StartTransmission,this,copy,trailer.GetRx2Freq(),trailer.GetRx2Dr(),powerIndex);
				}
				else{
					Simulator::Schedule(Seconds(trailer.GetDelay()-1),&LoRaRsGwNetDevice::StartTransmission,this,copy,frequency,GetRxDatarate(datarate,trailer.GetRx1Offset()),powerIndex);
				}
			}
		}

	void
		LoRaRsGwNetDevice::SendBeacon()
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
			for(uint8_t i = 0; i<m_availableChannels;i++)
			{
					beaconHeader.AddChannel(m_rssiValues[(i)%m_availableChannels],0x3f);
			}
			beacon->AddHeader(beaconHeader);
			StartTransmission(beacon,frequencies[(minutes+m_offset)%m_availableChannels],3,1);
			m_beacon = Simulator::Schedule(m_interBeacon,&LoRaRsGwNetDevice::SendBeacon,this);
		}

	void
		LoRaRsGwNetDevice::SetOffset (uint32_t offset)
		{
			m_offset = offset;
		}

	uint32_t 
		LoRaRsGwNetDevice::GetOffset ()
		{
			return m_offset;
		}

} // namespace ns3
