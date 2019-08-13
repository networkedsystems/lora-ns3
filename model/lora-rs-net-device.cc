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
#include <ns3/lora-phy.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/enum.h>
#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>
#include <ns3/channel.h>
#include <ns3/queue.h>
#include <ns3/trace-source-accessor.h>
#include "lora-mac-header.h"
#include "lora-mac-command.h"
#include "lora-rs-net-device.h"
#include <ns3/llc-snap-header.h>
#include <ns3/aloha-noack-mac-header.h>
#include <ns3/random-variable-stream.h>
#include <ns3/mobility-model.h>
namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("LoRaRsNetDevice");

  NS_OBJECT_ENSURE_REGISTERED (LoRaRsNetDevice);

  TypeId
  LoRaRsNetDevice::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::LoRaRsNetDevice")
				    .SetParent<LoRaNetDevice> ()
				    .AddConstructor<LoRaRsNetDevice> ()
						.AddAttribute ("Offset2", "Beacon offset",
								UintegerValue (0),
								MakeUintegerAccessor (&LoRaRsNetDevice::m_offset),//::SetOffset,
//									&LoRaRsNetDevice::GetOffset),
								MakeUintegerChecker<uint32_t> ())
						;
		return (tid);
	}

	LoRaRsNetDevice::LoRaRsNetDevice ()
		: LoRaNetDevice ()
	{
		NS_LOG_FUNCTION (this);
		m_seqNum = m_random->GetInteger(0,60);
		for (uint8_t index = 6; index < 16; index++){
			channelAvailable[index] = false;
			frequencies[index] = 0;
		}
		m_interBeacon = Seconds(60);
		m_rssiBeacon[0] = -150;
		m_rssiBeacon[1] = -175;
		m_rssiBeacon[2] = -175;
	}

	LoRaRsNetDevice::~LoRaRsNetDevice ()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaRsNetDevice::DoInitialize()
		{
			LoRaNetDevice::DoInitialize ();
			m_nextBeacon = Simulator::ScheduleNow(&LoRaRsNetDevice::ReceiveBeacon,this);
		}
	void
		LoRaRsNetDevice::DoDispose ()
		{
			NS_LOG_FUNCTION (this);
			m_nextBeacon.Cancel ();
			LoRaNetDevice::DoDispose ();
		}

	uint32_t 
		LoRaRsNetDevice::GetOffset ()
		{
			return m_offset;
		}

	void
		LoRaRsNetDevice::SetOffset (uint32_t offset)
		{
			m_offset = offset;
		}

	bool
		LoRaRsNetDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
		{
			NS_LOG_FUNCTION (packet << src << dest << protocolNumber);
			LoRaMacHeader header = LoRaMacHeader(LoRaMacHeader::LoRaMacType::LORA_MAC_UNCONFIRMED_DATA_UP,1);
			header.SetAddr (m_address);
			header.SetNoAck();
			//header.SetAdrAck ();
			packet->AddHeader (header);


			bool sendOk = true;
			NS_LOG_LOGIC (this << " state=" << m_state);
			Simulator::ScheduleNow(&LoRaNetDevice::TryAgain,this);
			NS_LOG_LOGIC ("enqueueing new packet");
			NS_ASSERT (m_queue);
			if (m_queue->Enqueue (Create<QueueItem> (packet)) == false)
			{
				m_macTxDropTrace (packet);
				sendOk = false;
			}
			return sendOk;

		}

	void
		LoRaRsNetDevice::DoPrepareReception (uint32_t bandwidthSetting, uint32_t frequency, uint32_t spreadingfactor)
		{
			NS_LOG_FUNCTION(this << bandwidthSetting << frequency << spreadingfactor);
			//std::cout << m_state << std::endl;
			NS_ASSERT(m_state!= RETRANSMISSION);
			NS_ASSERT(m_state!= TX);
			// RECOVER STATE after BEACON transmission
			if (m_state != RX1_PENDING && m_state != RX2_PENDING)
			{
				if(Simulator::GetDelayLeft(m_event2) == Seconds(0))
				{
					m_state = RX2_PENDING;
				}
				else
					m_state = RX1_PENDING;

			}
			if (Simulator::GetDelayLeft(m_beaconTimeout) <= 0)
			{
				if (m_state != BEACON && m_state!=RX)
				{
					Simulator::ScheduleNow(&LoRaPhy::SetBandwidth, m_phy, bandwidthSetting);
					Simulator::ScheduleNow(&LoRaPhy::SetChannelIndex, m_phy, frequency);
					Simulator::ScheduleNow(&LoRaPhy::SetSpreadingFactor, m_phy, spreadingfactor);
					Simulator::ScheduleNow(&LoRaPhy::ChangeState, m_phy, LoRaPhyState::LoRaRX);
					if (m_state == RX1_PENDING)
						m_event = Simulator::Schedule(Seconds(0.03),&LoRaRsNetDevice::CheckReception, this);
					else
						m_event2 = Simulator::Schedule(Seconds(0.03),&LoRaRsNetDevice::CheckReception2, this);
				}
				else if (m_state == BEACON && Simulator::GetDelayLeft(m_event2) == Seconds(0))
				{
					Simulator::ScheduleNow(&LoRaNetDevice::TryAgain, this);
				}
			}
		}


	void
		LoRaRsNetDevice::ReceiveBeacon()
		{
			NS_LOG_FUNCTION(this);
			uint32_t minutes = Simulator::Now().GetMinutes();
			m_state = BEACON;
			Simulator::ScheduleNow(&LoRaPhy::SetBandwidth, m_phy, 125000);
			Simulator::ScheduleNow(&LoRaPhy::SetChannelIndex, m_phy, frequencies[(m_offset+minutes)%3]);
			Simulator::ScheduleNow(&LoRaPhy::SetSpreadingFactor, m_phy, 9);
			Simulator::ScheduleNow(&LoRaPhy::ChangeState, m_phy, LoRaPhyState::LoRaRX);
			m_beaconTimeout = Simulator::Schedule(Seconds(0.4),&LoRaRsNetDevice::BeaconTimeout, this);
			//Simulator::Schedule(Seconds(0.4),&LoRaPhy::ChangeState,m_phy,LoRaPhyState::LoRaIDLE);
			m_nextBeacon = Simulator::Schedule(m_interBeacon,&LoRaRsNetDevice::ReceiveBeacon,this);
		}

	void
		LoRaRsNetDevice::BeaconTimeout (void)
		{
			NS_LOG_DEBUG(this);
			if (m_state!= RX)
			{
				m_state = IDLE;
				Simulator::ScheduleNow(&LoRaPhy::ChangeState,m_phy,LoRaPhyState::LoRaIDLE);
				if(m_currentPkt!= 0 && Simulator::GetDelayLeft(m_event2) <= Seconds(0) && m_channelRssiValues.size ()!=0)
				{
					// Circular shift for channel hopping
					//std::tuple<uint8_t,uint8_t> rssisf = m_channelRssiValues.front();
					//m_channelRssiValues.pop_front();
					//m_channelRssiValues.push_back(rssisf);
					m_transmission = Simulator::Schedule(Seconds(.6),&LoRaRsNetDevice::SchedulePacket,this,m_channelRssiValues,m_rssiBeacon[0]);
					m_state = TIMEOUT;
				}
			}
		}

	void
		LoRaRsNetDevice::DoCheckReception()
		{
			NS_LOG_FUNCTION(this);
			if (m_state != RX && m_state!= BEACON)
			{
				m_phy->ChangeState(LoRaPhyState::LoRaIDLE);
				m_state = RX2_PENDING;
			}
		}

	void
		LoRaRsNetDevice::DoTryAgain ()
		{
			NS_LOG_FUNCTION (this);
			if (m_state==IDLE || m_state==RETRANSMISSION || m_state==BEACON)
			{
				bool confirmed = false;
				if (m_currentPkt!=0)
				{
					LoRaMacHeader macheader;
					m_currentPkt->PeekHeader(macheader);
					confirmed = macheader.IsAck();
				}
				if (retransmissionCount == 9 && confirmed)
				{
					m_currentPkt = 0;
					m_transmission.Cancel();
					m_state = IDLE;
					missedCount++;
				}
				else if (retransmissionCount >= m_nbRep && !confirmed)
				{
					m_currentPkt = 0;
					m_transmission.Cancel();
					m_state = IDLE;
				}
				if(m_currentPkt==0)
				{
					if (m_queue->IsEmpty () == false)
					{
						startTimePacket = Simulator::Now();
						Ptr<QueueItem> item = m_queue->Dequeue ();
						NS_ASSERT(item);
						m_currentPkt = item->GetPacket ();
						LoRaMacHeader header2;
						m_currentPkt->RemoveHeader(header2);
						header2.SetFrmCounter(m_seqNum);
						if (m_ackCnt >= 60){
							header2.SetAdrAck ();
						}
						m_ackCnt++;
						m_seqNum++;
						while(!m_answers.empty())
						{
							header2.SetMacCommand (m_answers.front());
							m_answers.pop_front();
						}
						m_currentPkt->AddHeader(header2);
						NS_ASSERT (m_currentPkt);
						NS_LOG_LOGIC ("scheduling transmission now");
						m_state = TIMEOUT;
						retransmissionCount = 0;
					}
				}
				else if(confirmed && GetFreeChannel()!=127)
				{
					for (uint8_t i = 0; i<16;i++){
						if(datarate[i]>minDatarate[i] && (retransmissionCount == 3 || retransmissionCount == 5 || retransmissionCount == 7))
						{
							datarate[i]--;
						}
					}
					m_state = TIMEOUT;
				}
			}
		}

	void
		LoRaRsNetDevice::DoCheckReception2()
		{
			NS_LOG_FUNCTION(this);
			if (m_state != RX)
			{
				if (m_ackCnt > ADR_ACK_LIMIT)
				{
					for (uint8_t i = 0; i<16;i++){
						if(datarate[i]>minDatarate[i])
						{
							datarate[i]--;
						}
					}
					m_ackCnt = 0;
				}
				if (m_state != BEACON)
				{
					m_phy->ChangeState(LoRaPhyState::LoRaIDLE);
					m_state = RETRANSMISSION;
				}
				Simulator::ScheduleNow(&LoRaNetDevice::TryAgain, this);
			}
		}

	void
		LoRaRsNetDevice::NotifyReceptionStart ()
		{
			NS_LOG_FUNCTION (this);
			if (m_state != BEACON)
				m_state = RX;
		}

	void
		LoRaRsNetDevice::NotifyReceptionEndError ()
		{
			NS_LOG_FUNCTION (this);
			m_phy->ChangeState(LoRaPhyState::LoRaIDLE);
			// if it is the second reception window, we have to retransmit
			// otherwise we wait
			if(Simulator::GetDelayLeft(m_event2) <= NanoSeconds(0))
			{
				m_state=RETRANSMISSION;
				Simulator::ScheduleNow(&LoRaNetDevice::TryAgain, this);
				if (m_ackCnt > ADR_ACK_LIMIT)
				{
					for (uint8_t i = 0; i<16;i++){
						if(datarate[i]>minDatarate[i])
						{
							datarate[i]--;
						}
					}
					m_ackCnt = 0;
				}
			}
			else
			{
				m_state = RX2_PENDING;
			}
		}


	void
		LoRaRsNetDevice::NotifyReceptionEndOk (Ptr<Packet> packet, double rssi)
		{
			NS_LOG_FUNCTION (this << packet);
			LoRaMacHeader header;
			packet->RemoveHeader (header);
			NS_LOG_LOGIC ("packet : Gateway --> " << header.GetAddr () << " (here: " << m_address << ")");
			m_phy->ChangeState(LoRaPhyState::LoRaIDLE);

			PacketType packetType;
			if (header.GetAddr ().IsBroadcast ())
			{
				packetType = PACKET_BROADCAST;
			}
			else if (header.GetAddr ().IsGroup ())
			{
				packetType = PACKET_MULTICAST;
			}
			else if (header.GetAddr () == m_address)
			{
				packetType = PACKET_HOST;
			}
			else
			{
				packetType = PACKET_OTHERHOST;
			}

			NS_LOG_LOGIC ("packet type = " << packetType);

			if (!m_promiscRxCallback.IsNull ())
			{
				m_promiscRxCallback (this, packet->Copy (), 0, header.GetAddr (), header.GetAddr (), packetType);
			}

			if (packetType != PACKET_OTHERHOST && (header.GetDirection ()==FROMBASE))
			{
				if (!header.IsBeacon())
				{
					m_state = RETRANSMISSION;
					m_rxCallback (this, packet, 0 , header.GetAddr () );
					arrivedCount++;
					m_ackCnt = 0;
					averageTime = ((double)(arrivedCount-1)*averageTime+(double)(Simulator::Now().GetSeconds()-startTimePacket.GetSeconds()))/(double)arrivedCount;
					avgRetransmissionCount = ((double)(arrivedCount - 1)*(double)avgRetransmissionCount + retransmissionCount)/(double)arrivedCount;
					m_macRxTrace(packet);
					//remove packet, cause it has been received correctly.
					m_currentPkt = 0;
					m_transmission.Cancel();
					std::list<Ptr<LoRaMacCommand>> commands = header.GetCommandList ();
					for (std::list<Ptr<LoRaMacCommand> >::iterator it = commands.begin(); it!=commands.end(); ++it)
					{
						(*it)->Execute(this,m_address);
					}
					m_event2.Cancel();
					m_event.Cancel();
					Simulator::ScheduleNow(&LoRaNetDevice::TryAgain,this);
				}
				else
				{
					m_beaconTimeout.Cancel();
					NS_LOG_LOGIC("Reception is a beacon message" << Simulator::GetDelayLeft(m_event2));
					//m_rssiBeacon[GetChannelNbFromFrequency(m_phy->GetChannelIndex())] = .9*m_rssiBeacon[GetChannelNbFromFrequency(m_phy->GetChannelIndex())] + .1*m_phy->GetRssiLastPacket();
					m_rssiBeacon[0] = .9*m_rssiBeacon[0] + .1*m_phy->GetRssiLastPacket();
					m_channelRssiValues = header.GetChannels();
					if(m_currentPkt!= 0 && Simulator::GetDelayLeft(m_event2) <= Seconds(0))
					{
						m_transmission = Simulator::ScheduleNow(&LoRaRsNetDevice::SchedulePacket,this,header.GetChannels(),m_rssiBeacon[0]);
						//Simulator::ScheduleNow(&LoRaRsNetDevice::SchedulePacket,this,header.GetChannels(),m_rssiBeacon[GetChannelNbFromFrequency(m_phy->GetChannelIndex())]);
						m_state = TIMEOUT;
					}
				}
			}
			else
				if (Simulator::GetDelayLeft(m_event2) <= Seconds(0))
				{
					Simulator::ScheduleNow(&LoRaNetDevice::TryAgain,this);
					m_state = RETRANSMISSION;
				}
				else
					if (Simulator::GetDelayLeft(m_event2) > Seconds(1))
						m_state = RX1_PENDING;
					else
						m_state = RX2_PENDING;
		}

	void
		LoRaRsNetDevice::SchedulePacket (std::list< std::tuple<uint8_t,uint8_t> > channels, double rssi)
		{
			// select channel
			NS_LOG_FUNCTION(this << rssi);
			NS_ASSERT(m_currentPkt);
			uint8_t channelNb = 255;
			uint8_t channelNbMax = 255;
			std::tuple<uint8_t,uint8_t> channelSelected;
			std::tuple<uint8_t,uint8_t> channelMax = std::make_tuple(255,1);
			uint8_t index = 0;
			for (std::list<std::tuple<uint8_t,uint8_t> >::const_iterator it = channels.begin() ; it != channels.end() ; it++) {
				if (-1*((double)std::get<0>(*it)) > rssi-10*std::log10(0.025) + 10*log10(0.0016)-5)
				{
					if (channelNb == 255)
					{
						channelSelected = (*it);
						channelNb = index;
					}
					else if (std::get<0>(*it) > std::get<0>(channelSelected))
					{
						channelSelected = (*it);
						channelNb = index;
					}
				}
				if (std::get<0>(*it) < std::get<0>(channelMax))
				{
					channelNbMax = index;
					channelMax = (*it);
				}
				index++;
			}
			NS_LOG_DEBUG( -1*((double)std::get<0>(channelSelected)) << " " <<  rssi-10*std::log10(0.025) + 10*log10(0.0016)-5 << " "<<  rssi << " " << 10*std::log10(0.025) << " " << 10*log10(0.0016));
			//randomize spreading factor
			uint8_t sf = 12;
			if (channelNb != 255)
			{
				uint32_t throughput = 0;
				for (uint8_t i = 7; i< 13; i++)
				{
					if (((std::get<1>(channelSelected)>>(i-7)) & (0x01)) == 1)
						throughput += ((8192>>i)*i);
				}
				uint32_t threshold = m_random->GetInteger(0,throughput);
				uint32_t throughput2 = 0;
				for (uint8_t i = 7; i< 13; i++)
				{
					if (((std::get<1>(channelSelected)>>(i-7)) & (0x01)) == 1)
						throughput2 += ((8192>>i)*i);
					if (throughput2 >= threshold)
					{
						sf = i;
						break;
					}
				}
			}
			else
			{
				//select channel with highest rssi
				channelNb = channelNbMax;
				//select spreading factor 7
				sf = 7;
				channelSelected = channelMax;
			}
			NS_LOG_DEBUG("Transmit on channel " << (uint32_t)channelNb << "with spreading factor " << (uint32_t)sf);
			SetMaxDataRate (12-sf);
			SetChannelMask (0x0001<<(15-channelNb));

			// power control
			for (uint8_t i = 15; i>0;i--)
			{
				if (SetMaxPower (i))
					if (10*std::log10(power[i]) + rssi -10*std::log10(0.025) - 10.5 + 2.5*sf > -std::get<0>(channelSelected) + 9 )
						break;
			}
			NS_LOG_DEBUG(power[m_powerIndex] << " " <<  rssi << " " << 10*std::log10(power[m_powerIndex]) + rssi -10*std::log10(0.025) - 10.5 + 2.5*sf << " " << -std::get<0>(channelSelected));

			//randomize time
			Time offset = Seconds(100);
			Time timeToNextSlot = Simulator::GetDelayLeft(m_nextBeacon);
			while ((offset+m_phy->GetTimeOfPacket(m_currentPkt->GetSize(),sf)) > timeToNextSlot)
				offset = Seconds(m_random->GetInteger(0,60))+MilliSeconds(m_random->GetInteger(0,999))+MicroSeconds(m_random->GetInteger(0,999))+NanoSeconds(m_random->GetInteger(0,999));
			m_event = Simulator::Schedule(offset,&LoRaRsNetDevice::StartTransmissionNoArgs, this);
		}



	void LoRaRsNetDevice::SetReset (bool reset)
	{
		m_reset = reset;
	}


	uint8_t
		LoRaRsNetDevice::GetChannelNbFromFrequency(uint32_t frequency)
		{
			for(uint8_t i = 0; i<16; i++)
			{
				if (frequencies[i] == frequency)
					return i;
			}
			return 255;
		}

} // namespace ns3
