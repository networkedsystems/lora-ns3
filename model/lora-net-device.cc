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
#include <ns3/link-check-req.h>
#include "lora-net-device.h"
#include "ns3/llc-snap-header.h"
#include "ns3/aloha-noack-mac-header.h"
#include <ns3/random-variable-stream.h>
namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaNetDevice");

	std::ostream& operator<< (std::ostream& os, LoRaNetDevice::State state)
	{
		switch (state)
		{
			case LoRaNetDevice::IDLE:
				os << "IDLE";
				break;
			case LoRaNetDevice::TIMEOUT:
				os << "TIMEOUT";
				break;
			case LoRaNetDevice::TX:
				os << "TX";
				break;
			case LoRaNetDevice::RX:
				os << "RX";
				break;
			case LoRaNetDevice::RETRANSMISSION:
				os << "RETRANSMISSION";
				break;
			case LoRaNetDevice::RX1_PENDING:
				os << "RX1_PENDING";
				break;
			case LoRaNetDevice::RX2_PENDING:
				os << "RX2_PENDING";
				break;
			case LoRaNetDevice::BEACON:
				os << "BEACON";
				break;
			default:
				os << "UNKNOWN";
		}
		return os;
	}


	NS_OBJECT_ENSURE_REGISTERED (LoRaNetDevice);

	TypeId
		LoRaNetDevice::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaNetDevice")
				.SetParent<NetDevice> ()
				.AddConstructor<LoRaNetDevice> ()
				.AddAttribute ("Address",
						"The MAC address of this device.",
						Mac32AddressValue (Mac32Address ("00:00:00:01")),
						MakeMac32AddressAccessor (&LoRaNetDevice::m_address),
						MakeMac32AddressChecker ())
				.AddAttribute ("Queue",
						"packets being transmitted get queued here",
						PointerValue (),
						MakePointerAccessor (&LoRaNetDevice::m_queue),
						MakePointerChecker<Queue> ())
				.AddAttribute ("Mtu", "The Maximum Transmission Unit",
						UintegerValue (255),
						MakeUintegerAccessor (&LoRaNetDevice::SetMtu,
							&LoRaNetDevice::GetMtu),
						MakeUintegerChecker<uint16_t> (1,65535))
				.AddAttribute ("Phy", "The PHY layer attached to this device.",
						PointerValue (),
						MakePointerAccessor (&LoRaNetDevice::GetPhy,
							&LoRaNetDevice::SetPhy),
						MakePointerChecker<Object> ())
				.AddTraceSource ("MacTx",
						"Trace source indicating a packet has arrived "
						"for transmission by this device",
						MakeTraceSourceAccessor (&LoRaNetDevice::m_macTxTrace),
						"ns3::Packet::TracedCallback")
				.AddTraceSource ("MacTxDrop",
						"Trace source indicating a packet has been dropped "
						"by the device before transmission",
						MakeTraceSourceAccessor (&LoRaNetDevice::m_macTxDropTrace),
						"ns3::Packet::TracedCallback")
				.AddTraceSource ("MacPromiscRx",
						"A packet has been received by this device, has been "
						"passed up from the physical layer "
						"and is being forwarded up the local protocol stack.  "
						"This is a promiscuous trace,",
						MakeTraceSourceAccessor (&LoRaNetDevice::m_macPromiscRxTrace),
						"ns3::Packet::TracedCallback")
				.AddTraceSource ("MacRx",
						"A packet has been received by this device, "
						"has been passed up from the physical layer "
						"and is being forwarded up the local protocol stack.  "
						"This is a non-promiscuous trace,",
						MakeTraceSourceAccessor (&LoRaNetDevice::m_macRxTrace),
						"ns3::Packet::TracedCallback")
				;
			return tid;
		}

	LoRaNetDevice::LoRaNetDevice ()
		: m_state (IDLE)
	{
		NS_LOG_FUNCTION (this);
		m_random=CreateObject<UniformRandomVariable> ();
		m_seqNum =0;
		m_currentPkt = 0;
		m_dutyCycle = 7;
		m_waitingFactor = 99;//(1.0-pow(2.0,-(double)m_dutyCycle))/pow(2.0,-(double)m_dutyCycle);
		m_delay = 1;
		retransmissionCount = 0;
		m_powerIndex = 1;
		m_nbRep = 1;
		m_ackCnt = m_random->GetInteger(1,60);
		avgRetransmissionCount = 0;
		missedCount = 0;
		arrivedCount=0;
		averageTime = 0;
		m_node = 0;
		m_rx2Datarate = 0; 
		m_rx2Freq = 8695250;
		m_rx1Offset = 0;
	}

	LoRaNetDevice::~LoRaNetDevice ()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaNetDevice::DoDispose ()
		{
			NS_LOG_FUNCTION (this);
			m_queue = 0;
			m_node = 0;
			m_channel = 0;
			m_currentPkt = 0;
			m_phy = 0;
			m_random = 0;
			m_phyMacTxStartCallback = MakeNullCallback< bool, Ptr<Packet> > ();
			m_rxCallback = MakeNullCallback <bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address& > ();
			m_promiscRxCallback = MakeNullCallback <bool, Ptr<NetDevice>,Ptr<const Packet>, uint16_t, const Address&, const Address&, PacketType > ();
			NetDevice::DoDispose ();
		}


	void
		LoRaNetDevice::SetIfIndex (const uint32_t index)
		{
			NS_LOG_FUNCTION (index);
			m_ifIndex = index;
		}

	uint32_t
		LoRaNetDevice::GetIfIndex (void) const
		{
			NS_LOG_FUNCTION (this);
			return m_ifIndex;
		}

	double
		LoRaNetDevice::GetAvgRetransmissionCount()
		{
			return avgRetransmissionCount;
		}

	double
		LoRaNetDevice::GetAvgTime ()
		{
			return averageTime;
		}

uint32_t
	LoRaNetDevice::GetMissed ()
	{
		return missedCount;
	}

uint32_t
	LoRaNetDevice::GetArrived ()
	{
		return arrivedCount;
	}

	bool
		LoRaNetDevice::SetMtu (uint16_t mtu)
		{
			NS_LOG_FUNCTION (mtu);
			m_mtu = mtu;
			return true;
		}

	uint16_t
		LoRaNetDevice::GetMtu (void) const
		{
			NS_LOG_FUNCTION (this);
			return m_mtu;
		}


	void
		LoRaNetDevice::SetQueue (Ptr<Queue> q)
		{
			NS_LOG_FUNCTION (q);
			m_queue = q;
		}


	void
		LoRaNetDevice::SetAddress (Address address)
		{
			NS_LOG_FUNCTION (this);
			m_address = Mac32Address::ConvertFrom (address);
		}

	Address
		LoRaNetDevice::GetAddress (void) const
		{
			NS_LOG_FUNCTION (this);
			return m_address;
		}

	bool
		LoRaNetDevice::IsBroadcast (void) const
		{
			NS_LOG_FUNCTION (this);
			return false;
		}

	Address
		LoRaNetDevice::GetBroadcast (void) const
		{
			NS_LOG_FUNCTION (this);
			return Mac32Address ("00:00:00:01");
		}

	bool
		LoRaNetDevice::IsMulticast (void) const
		{
			NS_LOG_FUNCTION (this);
			return false;
		}

	Address
		LoRaNetDevice::GetMulticast (Ipv4Address addr) const
		{
			NS_LOG_FUNCTION (addr);
			Mac32Address ad = Mac32Address::GetMulticast (addr);
			return ad;
		}


	Address LoRaNetDevice::GetMulticast (Ipv6Address addr) const
	{
		NS_LOG_FUNCTION (addr);
		Mac32Address ad = Mac32Address::GetMulticast (addr);
		return ad;
	}


	bool
		LoRaNetDevice::IsPointToPoint (void) const
		{
			NS_LOG_FUNCTION (this);
			return false;
		}

	bool
		LoRaNetDevice::IsBridge (void) const
		{
			NS_LOG_FUNCTION (this);
			return false;
		}


	Ptr<Node>
		LoRaNetDevice::GetNode (void) const
		{
			NS_LOG_FUNCTION (this);
			return m_node;
		}

	void
		LoRaNetDevice::SetNode (Ptr<Node> node)
		{
			NS_LOG_FUNCTION (node);

			m_node = node;
		}

	void
		LoRaNetDevice::SetPhy (Ptr<LoRaPhy> phy)
		{
			NS_LOG_FUNCTION (this << phy);
			m_phy = phy;
		}


	Ptr<LoRaPhy>
		LoRaNetDevice::GetPhy () const
		{
			NS_LOG_FUNCTION (this);
			return m_phy;
		}


	void
		LoRaNetDevice::SetChannel (Ptr<Channel> c)
		{
			NS_LOG_FUNCTION (this << c);
			m_channel = c;
		}


	Ptr<Channel>
		LoRaNetDevice::GetChannel (void) const
		{
			NS_LOG_FUNCTION (this);
			return m_channel;
		}


	bool
		LoRaNetDevice::NeedsArp (void) const
		{
			NS_LOG_FUNCTION (this);
			return true;
		}

	bool
		LoRaNetDevice::IsLinkUp (void) const
		{
			NS_LOG_FUNCTION (this);
			return m_linkUp;
		}

	void
		LoRaNetDevice::AddLinkChangeCallback (Callback<void> callback)
		{
			NS_LOG_FUNCTION (&callback);
			m_linkChangeCallbacks.ConnectWithoutContext (callback);
		}

	void
		LoRaNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
		{
			NS_LOG_FUNCTION (&cb);
			m_rxCallback = cb;
		}

	void
		LoRaNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
		{
			NS_LOG_FUNCTION (&cb);
			m_promiscRxCallback = cb;
		}

	bool
		LoRaNetDevice::SupportsSendFrom () const
		{
			NS_LOG_FUNCTION (this);
			return true;
		}

	bool
		LoRaNetDevice::Send (Ptr<Packet> packet,uint16_t protocolNumber)
		{
			NS_LOG_FUNCTION (packet);
			return this->SendFrom (packet, m_address, m_address, protocolNumber);
		}

	bool
		LoRaNetDevice::Send (Ptr<Packet> packet,const Address& dest)
		{
			NS_LOG_FUNCTION (packet << dest);
			return this->SendFrom (packet, m_address, dest, 0 );
		}

	bool
		LoRaNetDevice::Send (Ptr<Packet> packet,const Address& dest, uint16_t protocolNumber)
		{
			NS_LOG_FUNCTION (packet << dest << protocolNumber);
			return this->SendFrom (packet, m_address, dest, protocolNumber);
		}

	bool
		LoRaNetDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
		{
			// ignore all destination addresses, all packets are for base station anyway, but keep to be compatible with NetDevice
			// also the address field is just this device. 
			NS_LOG_FUNCTION (packet << src << dest << protocolNumber);
			LoRaMacHeader header = LoRaMacHeader(LoRaMacHeader::LoRaMacType::LORA_MAC_CONFIRMED_DATA_UP,1);
			header.SetAddr (m_address);
			header.SetNoAck();
			header.SetPort (protocolNumber);
			if (m_seqNum%10 == 4)
			{
				header.SetMacCommand(CreateObject <LinkCheckReq> ());
			}
			packet->AddHeader (header);


			bool sendOk = true;
			//
			// If the device is idle, transmission starts immediately. Otherwise,
			// the transmission will be started by NotifyTransmissionEnd
			//
			NS_LOG_LOGIC (this << " state=" << m_state);
			if (m_state == IDLE)
			{
				Simulator::Remove(m_event);
				Simulator::ScheduleNow(&LoRaNetDevice::TryAgain,this);
				NS_LOG_LOGIC ("enqueueing new packet");
				if (m_queue->Enqueue (Create<QueueItem> (packet)) == false)
				{
					m_macTxDropTrace (packet);
					sendOk = false;
				}
			}
			else
			{
				NS_LOG_LOGIC ("deferring TX, enqueueing new packet");
				NS_ASSERT (m_queue);
				if (m_queue->Enqueue (Create<QueueItem> (packet)) == false)
				{
					m_macTxDropTrace (packet);
					sendOk = false;
				}
			}
			return sendOk;

		}

	void
		LoRaNetDevice::SetGenericPhyTxStartCallback (GenericPhyTxStartCallback c)
		{
			NS_LOG_FUNCTION (this);
			m_phyMacTxStartCallback = c;
		}

	uint8_t
		LoRaNetDevice::GetFreeChannel()
		{
	  	int chan = m_random->GetInteger(0,2);
			uint8_t it = 0;
			while(it < 16)
			{
				if (channelAvailable[chan%16])
					return chan%16;
				chan+=7;
				it++;
			}
		return 127;
		}

	void
		LoRaNetDevice::StartTransmissionNoArgs ()
		{
			//if (m_ackCnt>60)
			NS_LOG_FUNCTION (this);
			NS_ASSERT (m_currentPkt != 0);
			//NS_ASSERT (m_state == IDLE || m_state == RETRANSMISSION);
			NS_ASSERT (m_state == TIMEOUT);
			// Choose channel
			uint16_t channel = GetFreeChannel();
			// This should not be the case, but anyway.
			if (channel!=127)
			{
				m_channelIndex = channel;
				// Set parameters of phy device
				if (StartTransmission (m_currentPkt->Copy(), frequencies[channel], datarate[channel], m_powerIndex))
				{
					retransmissionCount++;
					m_state = TX;
					m_macTxTrace(m_currentPkt);
					LastSend [channel] = Simulator::Now().GetSeconds();
				}
			}
			else
			{
				// No channel has been found, so delay decision again.
				m_state = RETRANSMISSION;
				m_event = Simulator::Schedule(Seconds(1+m_random->GetInteger(0,1))+MilliSeconds(m_random->GetInteger(0,999))+MicroSeconds(m_random->GetInteger(0,999))+NanoSeconds(m_random->GetInteger(0,999)),&LoRaNetDevice::TryAgain, this);
			}
		}
	
	bool
		LoRaNetDevice::StartTransmission (Ptr<Packet> packet, uint32_t frequency, uint8_t datarate, uint8_t powerIndex)
		{
			NS_LOG_FUNCTION (this << frequency << (uint32_t)datarate << (uint32_t)powerIndex);
			// Set parameters of phy device
			m_phy->SetChannelIndex(frequency);
			m_phy->SetPower(power[powerIndex]);
			m_phy->SetBandwidth(bandwidth[datarate]);
			m_phy->SetSpreadingFactor(spreading[datarate]);
			m_phy->ChangeState(LoRaPhy::State::TX);
			// Start transmission
			if (!m_phyMacTxStartCallback (packet->Copy()))
			{
				NS_LOG_WARN ("PHY refused to start TX");
				return false;
			}
			else
			{
				return true;
			}
		}

	void
		LoRaNetDevice::FreeChannel (uint8_t channelIndex)
		{
			NS_LOG_FUNCTION ((uint32_t) channelIndex);
			channelAvailable[channelIndex] = true;
		}

	bool
		LoRaNetDevice::SetMaxPower (uint8_t maxPower)
		{
			double tempPower = power [maxPower];
			if (tempPower != 0)
			{
				m_powerIndex = maxPower;
				return true;
			}
			return false;
		}

	bool
		LoRaNetDevice::SetMaxDataRate (uint8_t maxSetting)
		{
			NS_LOG_FUNCTION((uint32_t)maxSetting);
			NS_ASSERT (maxSetting < 0x0F);
			for (uint8_t i=0; i<16; i++)
			{
				datarate[i] = maxSetting;
				maxDatarate[i] = maxSetting;
			}
			return true;
		}
	
	bool
		LoRaNetDevice::SetMaxDataRate (uint8_t maxSetting, uint8_t index)
		{
			NS_LOG_FUNCTION((uint32_t)maxSetting << (uint32_t) index);
			NS_ASSERT (maxSetting < 0x0F);
			datarate[index] = maxSetting;
			maxDatarate[index] = maxSetting;
			return true;
		}
	
	bool
		LoRaNetDevice::SetMinDataRate (uint8_t minSetting)
		{
			for (uint8_t i=0; i<16; i++)
			{
				minDatarate[i] = minSetting;
			}
			return true;
		}
	bool 
		LoRaNetDevice::SetDelay (uint8_t delay)
		{
			m_delay = delay;
			return true;
		}

	bool
		LoRaNetDevice::AddChannel (uint8_t index, uint32_t freq)
		{
			NS_LOG_FUNCTION (index << freq);
			frequencies[index] = freq;
			channelAvailable [index] = true;
			return true;
		}
	
	void 
		LoRaNetDevice::RemoveChannel (uint8_t index)
		{
			NS_LOG_FUNCTION (index);
			channelAvailable [index] = false;
			m_freeChannel.Cancel();
		}

	void 
		LoRaNetDevice::CheckCorrectReceiver (LoRaMacHeader header)
		{
			NS_LOG_FUNCTION(header.GetAddr());
			// if it is for this node, drop everything.
			// even if it fails, there won't be another message
			if (header.GetAddr()==m_address)
			{
				Simulator::Remove(m_event2);
				Simulator::Remove(m_event);
			}
			// If it is not for this node, stop reception
			else 
			{
				// if this is already after the final event, retransmit
				if(Simulator::GetDelayLeft(m_event2) <= NanoSeconds(0))
				{
					m_state = RETRANSMISSION;
					m_event = Simulator::ScheduleNow(&LoRaNetDevice::TryAgain,this);
				}
				// else check if this node is still handling the first receive window				
				else
				{
					if(Simulator::GetDelayLeft(m_event) <= NanoSeconds(0))
					{
						m_state = RX2_PENDING;
						Simulator::Remove(m_event);
					}
					else
					{
						m_state = RX1_PENDING;
					}
				}
				m_phy->ChangeState(LoRaPhy::State::IDLE);
			}
		}

	void
		LoRaNetDevice::NotifyTransmissionEnd (Ptr<const Packet>)
		{
			NS_LOG_FUNCTION (this);
			m_state = RX1_PENDING;
			NS_ASSERT (m_queue);
			channelAvailable [m_channelIndex] = false;
			NS_LOG_DEBUG(m_channelIndex << (uint32_t)spreading[m_channelIndex]);
			double timeNow = Simulator::Now().GetSeconds();
			m_freeChannel = Simulator::Schedule(Seconds((timeNow-LastSend[m_channelIndex])*m_waitingFactor),&LoRaNetDevice::FreeChannel,this,m_channelIndex);
			m_event = Simulator::Schedule(Seconds(m_delay-0.01),&LoRaNetDevice::PrepareReception, this, bandwidth[datarate[m_channelIndex]], frequencies[m_channelIndex],spreading[datarate[m_channelIndex]]);
			m_event2 = Simulator::Schedule(Seconds(m_delay+0.99),&LoRaNetDevice::PrepareReception, this, bandwidth[m_rx2Datarate], m_rx2Freq,spreading[m_rx2Datarate]);
		}
	
	void
		LoRaNetDevice::DoPrepareReception (uint32_t bandwidthSetting, uint32_t frequency, uint32_t spreadingfactor)
		{
			NS_LOG_FUNCTION(this << bandwidthSetting << frequency << spreadingfactor);
			NS_ASSERT(m_state!= RETRANSMISSION);
			NS_ASSERT(m_state!= TX);
			//NS_ASSERT(m_state!= RX);
			if (m_state == RX)
			{
				// ignore this case, because this means that some packet is being received at the moment
				// usually one with a high spreading factor
				// wait a bit longer.
			}
			else
			{
				Simulator::ScheduleNow(&LoRaPhy::SetBandwidth, m_phy, bandwidthSetting);
				Simulator::ScheduleNow(&LoRaPhy::SetChannelIndex, m_phy, frequency);
				Simulator::ScheduleNow(&LoRaPhy::SetSpreadingFactor, m_phy, spreadingfactor);
				Simulator::ScheduleNow(&LoRaPhy::ChangeState, m_phy, LoRaPhy::State::RX);
				if (m_state == RX1_PENDING)
					m_event = Simulator::Schedule(Seconds(0.03),&LoRaNetDevice::CheckReception, this);
				else
					m_event2 = Simulator::Schedule(Seconds(0.03),&LoRaNetDevice::CheckReception2, this);
			}
		}

	void
		LoRaNetDevice::PrepareReception (uint32_t bandwidthSetting, uint32_t frequency, uint32_t spreadingfactor)
		{
			this->DoPrepareReception (bandwidthSetting, frequency, spreadingfactor);
		}


	void 
		LoRaNetDevice::CheckReception()
		{
			this->DoCheckReception ();
		}
	void 
		LoRaNetDevice::DoCheckReception()
		{
			NS_LOG_FUNCTION(this);
			NS_ASSERT (m_state == RX || m_state == RX1_PENDING);
			if (m_state != RX )
			{
				m_phy->ChangeState(LoRaPhy::State::IDLE);
				m_state = RX2_PENDING;
			}
		}

	void
		LoRaNetDevice::TryAgain ()
		{
			this->DoTryAgain ();
		}

	void
		LoRaNetDevice::DoTryAgain ()
		{
			NS_LOG_FUNCTION (this);
			Simulator::Remove(m_event);
			if (m_state==IDLE || m_state==RETRANSMISSION)
			{
				bool confirmed = false;
				if (m_currentPkt!=0)
				{
					LoRaMacHeader macheader;
					m_currentPkt->PeekHeader(macheader);
					confirmed = macheader.NeedsAck();
				}
				if (retransmissionCount == 9 && confirmed)
				{
					m_currentPkt = 0;
					m_state = IDLE;
					missedCount++;
				}
				else if (retransmissionCount >= m_nbRep && !confirmed)
				{
					m_currentPkt = 0;
					m_state = IDLE;
				}
				// Get new message from the queue
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
						retransmissionCount = 0;
						m_state = TIMEOUT;
						m_event = Simulator::Schedule(Seconds(1)+MilliSeconds(m_random->GetInteger(0,999))+MicroSeconds(m_random->GetInteger(0,999))+NanoSeconds(m_random->GetInteger(0,999)),&LoRaNetDevice::StartTransmissionNoArgs, this);
					}
				}
				// retransmit previous message
				else if(confirmed && GetFreeChannel()!=127)
				{
					for (uint8_t i = 0; i<16;i++){
						if(datarate[i]>minDatarate[i] && (retransmissionCount == 3 || retransmissionCount == 5 || retransmissionCount == 7))
						{
							datarate[i]--;
						}
					}
					m_state = TIMEOUT;
					m_event = Simulator::Schedule(Seconds(1+m_random->GetInteger(0,1))+MilliSeconds(m_random->GetInteger(0,999))+MicroSeconds(m_random->GetInteger(0,999))+NanoSeconds(m_random->GetInteger(0,999)),&LoRaNetDevice::StartTransmissionNoArgs, this);
				}
				// In case there is no free channel, delay the decision to a future time.
				else
					m_event = Simulator::Schedule(Seconds(1),&LoRaNetDevice::TryAgain,this);
			}
		}

	void 
		LoRaNetDevice::CheckReception2()
		{
			this->DoCheckReception2 ();
		}
	void 
		LoRaNetDevice::DoCheckReception2()
		{
			NS_LOG_FUNCTION(this);
			NS_ASSERT (m_state == RX || m_state == RX2_PENDING);
			if (m_state != RX)
			{
				m_state = RETRANSMISSION;
				m_phy->ChangeState(LoRaPhy::State::IDLE);
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
			m_event = Simulator::ScheduleNow(&LoRaNetDevice::TryAgain, this);
		}
	void
		LoRaNetDevice::NotifyReceptionStart ()
		{
			NS_LOG_FUNCTION (this);
			m_state = RX;
		}

	void
		LoRaNetDevice::NotifyReceptionEndError ()
		{
			NS_LOG_FUNCTION (this);
			m_phy->ChangeState(LoRaPhy::State::IDLE);
			// if it is the second reception window, we have to retransmit
			// otherwise we wait
			if(Simulator::GetDelayLeft(m_event2) <= NanoSeconds(0))
			{
				// go to idle only with second slot
				m_state=RETRANSMISSION;
				m_event = Simulator::ScheduleNow(&LoRaNetDevice::TryAgain, this);
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
				Simulator::Remove(m_event);
			}
		}


	void
		LoRaNetDevice::NotifyReceptionEndOk (Ptr<Packet> packet, double per)
		{
			NS_LOG_FUNCTION (this << packet << per);

			LoRaMacHeader header;
			packet->RemoveHeader (header);
			NS_LOG_LOGIC ("packet : Gateway --> " << header.GetAddr () << " (here: " << m_address << ")");
			m_phy->ChangeState(LoRaPhy::State::IDLE);

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

			NS_LOG_LOGIC ("packet type = " << packetType << " " << PACKET_HOST << " " << header.IsAcknowledgment () << " " << header.GetDirection () << FROMBASE);

			if (!m_promiscRxCallback.IsNull ())
			{
				m_promiscRxCallback (this, packet->Copy (), 0, header.GetAddr (), header.GetAddr (), packetType);
			}

			if (packetType != PACKET_OTHERHOST && (header.GetDirection ()==FROMBASE))
			{
				Simulator::Remove(m_event2);
				Simulator::Remove(m_event);
				m_state = RETRANSMISSION;
				NS_LOG_DEBUG( "Got Ack with spreading " << (uint32_t)spreading[datarate[m_channelIndex]]);
				m_rxCallback (this, packet, 0 , header.GetAddr () );
				arrivedCount++;
				m_ackCnt = 0;
				averageTime = ((double)(arrivedCount-1)*averageTime+(double)(Simulator::Now().GetSeconds()-startTimePacket.GetSeconds()))/(double)arrivedCount;
				avgRetransmissionCount = ((double)(arrivedCount - 1)*(double)avgRetransmissionCount + retransmissionCount)/(double)arrivedCount;
				m_macRxTrace(packet);
				if (header.IsAcknowledgment ())
					m_currentPkt = 0;
				std::list<Ptr<LoRaMacCommand>> commands = header.GetCommandList ();
				for (std::list<Ptr<LoRaMacCommand>>::iterator it = commands.begin(); it!=commands.end(); ++it)
				{
					(*it)->Execute(this,m_address);
				}
				m_event = Simulator::ScheduleNow(&LoRaNetDevice::TryAgain, this);
			}
			else
			{
				if(Simulator::GetDelayLeft(m_event2) <= NanoSeconds(0))
				{
					m_state = RETRANSMISSION;
					m_event = Simulator::ScheduleNow(&LoRaNetDevice::TryAgain, this);
				}
				else
				{
					Simulator::Remove(m_event);
					m_state = RX2_PENDING;
				}
			}
		}

	void 
		LoRaNetDevice::SetNbRep (uint8_t repetitions)
		{
			m_nbRep = repetitions;
		}

	bool
		LoRaNetDevice::SetChannelMask (uint16_t channelMask)
		{
			NS_LOG_DEBUG(channelMask);
			m_freeChannel.Cancel();
			uint8_t it = 0;
			for(uint16_t i = 0; i<16; i++)
			{
				if ((channelMask >> (15-i))&0x01)
				{
					if (frequencies[i]!=0)
					{
						it++;
					}
				}
			}
			if (it>0)
			{
				for(uint16_t i = 0; i<16; i++)
				{
					if ((channelMask >> (15-i))&0x01)
					{
						if (frequencies[i]!=0)
						{
							channelAvailable [i] = true;
						}
						else
							return false;
					}
					else
						channelAvailable[i] = false;
				}
				return true;
			}
			else
			{
				return false;
			}
		}

	void
		LoRaNetDevice::SetMacAnswer (Ptr<LoRaMacCommand> command)
		{
			m_answers.push_back(command);
		}

	void 
		LoRaNetDevice::SetDutyCycle(uint8_t dutyCycle)
		{
			NS_LOG_FUNCTION (this << dutyCycle);
			m_dutyCycle = dutyCycle;
			m_waitingFactor = pow(2.0,(double)m_dutyCycle)-1;
		}

bool LoRaNetDevice::SetRx2Settings(uint8_t datarate, uint32_t freq)
{
	m_rx2Datarate = datarate;
	m_rx2Freq = freq;
	return true;
}

uint32_t LoRaNetDevice::GetRx2Frequency ()
{
	return m_rx2Freq;
}

uint8_t LoRaNetDevice::GetRx2Datarate ()
{
	return m_rx2Datarate;
}

bool LoRaNetDevice::SetDlOffset (uint8_t offset)
{
	m_rx1Offset = offset;
	return true;
}

uint8_t
	LoRaNetDevice::GetDlOffset ()
{
	return m_rx1Offset;
}

uint8_t 
LoRaNetDevice::GetRxDatarate (uint8_t dr)
{
	if (dr > m_rx1Offset)
		return dr - m_rx1Offset;
	else
		return 0;
}

} // namespace ns3
