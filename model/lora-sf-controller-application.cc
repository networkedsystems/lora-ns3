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

#include "lora-application.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "lora-sf-controller-application.h"
#include "lora-mac-header.h"
#include "commands/new-channel-req.h"
#include "commands/new-channel-ans.h"
#include "lora-mac-command.h"
#include "gw-trailer.h"
#include <bitset>
#include <experimental/random>
namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaSfControllerApplication");

	NS_OBJECT_ENSURE_REGISTERED (LoRaSfControllerApplication);

	// Application Methods

	TypeId 
		LoRaSfControllerApplication::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaSfControllerApplication")
				.AddConstructor<LoRaSfControllerApplication>()
				.SetParent<LoRaNetworkApplication> ()
				.SetGroupName("LoRa")
				;
			return tid;
		}

	// \brief Application Constructor
	LoRaSfControllerApplication::LoRaSfControllerApplication()
	{
		NS_LOG_FUNCTION (this);
		for (uint8_t f = 0; f < 3; f++)
			for (uint8_t i = 0; i < 10; i++)
				m_bandits[f][i] = new SfMab(3,5);
	}

	// \brief LoRaSfControllerApplication Destructor
	LoRaSfControllerApplication::~LoRaSfControllerApplication()
	{
		NS_LOG_FUNCTION (this);
		for (uint8_t f = 0; f < 3; f++)
			for (uint8_t i = 0; i < 10; i++)
				delete m_bandits[f][i];

	}

	void
		LoRaSfControllerApplication::DoDispose (void)
		{
			NS_LOG_FUNCTION (this);
			m_data.clear();
			m_settings.clear();
		}

	void
		LoRaSfControllerApplication::DoInitialize (void)
		{
			NS_LOG_FUNCTION (this);
			Application::DoInitialize ();
		}

	// Protected methods
	// StartApp and StopApp will likely be overridden by application subclasses
	void LoRaSfControllerApplication::StartApplication ()
	{ // Provide null functionality in case subclass is not interested
		NS_LOG_FUNCTION (this);
		m_settingCalculation = Simulator::Schedule(Seconds(20*60),&LoRaSfControllerApplication::CalculateSetting,this);

	}

	void LoRaSfControllerApplication::StopApplication ()
	{ // Provide null functionality in case subclass is not interested
		NS_LOG_FUNCTION (this);
		m_settingCalculation.Cancel();
	}

	//std::tuple<uint16_t, uint8_t,uint8_t>
	//	LoRaSfControllerApplication::GetSetting (const Address& address)
	//	{
	//		return m_settings[address];
	//		//for (auto &jt : m_settings)
	//		//{
	//		//	if (address == std::get<0>(jt))
	//		//	{
	//		//		std::cout << "datarate " << (uint32_t)std::get<2>(jt) << " and power  " << (uint32_t)std::get<3>(jt) << std::endl;
	//		//		return std::make_tuple(std::get<1>(jt),std::get<2>(jt),std::get<3>(jt));
	//		//	}
	//		//}
	//		//return std::make_tuple(0,0,0);
	//	}

	void
		LoRaSfControllerApplication::SaveSetting (const Address& address, uint8_t spreadingFactors[3])
		{
			NS_LOG_FUNCTION (this << address << (uint32_t)spreadingFactors[0] << (uint32_t)spreadingFactors[1] << (uint32_t)spreadingFactors[2]);
			Setting s;
			for (uint8_t i = 0; i< 3; i++)
				s.sfs[i] = spreadingFactors[i];
			m_settings[address] = s; 
		}
	void
		LoRaSfControllerApplication::ConfirmPower (const Address& address)
		{
			NS_LOG_FUNCTION (this << address);
		}

	bool
		LoRaSfControllerApplication::CheckTuple(const TableValue& first,const TableValue& second)
		{
			return first.rssi > second.rssi;
		}



	void
		LoRaSfControllerApplication::CalculateSetting (void)
		{
			NS_LOG_FUNCTION (this);
			m_lastChannel = ((uint32_t)Simulator::Now().GetMinutes())%3;
			Simulator::Schedule(Seconds(10*60),&LoRaSfControllerApplication::CalculateSetting,this);
			//Create array for sorting
			std::list<TableValue> list;
			for (std::map<Address,TableValue>::iterator it = m_data.begin(); it!=m_data.end(); ++it)
			{
				list.push_back(it->second);
			}
			//SORT based on rssi
			list.sort(LoRaSfControllerApplication::CheckTuple);

			// divide group in 10, for each group apply bandit i and for each frequency
			//for (uint8_t f = 0; f<3; f++)
			uint8_t f = m_lastChannel;
			{
				// update observation
				double totalPackets[10]; 
				double totalReceived[10];
				for (uint8_t i = 0; i<10; i++)
				{
					totalPackets[i]=0;
					totalReceived[i]=0;
				}
				uint32_t index = 0;
				for (std::list<TableValue>::iterator it = list.begin(); it!=list.end(); it++)
				{
					uint32_t indexVal = 10*index/list.size();
					totalPackets[indexVal] += (it->lastPacketNumber - it->lastValue[f]);
					totalReceived[indexVal] += it->received[f];
					m_data[it->addr].lastValue[f] = m_data[it->addr].lastPacketNumber; 
					m_data[it->addr].received[f] = 0; 
					NS_LOG_INFO( it->addr << " " <<  (uint32_t)totalReceived[indexVal] << " " << (uint32_t)totalPackets[indexVal]);
					index++;
				}
				uint8_t sf[10];
				double observation[10];
				for (uint8_t i = 0; i<10; i++)
				{
					NS_LOG_INFO((uint32_t)totalReceived[i] << " " <<  (uint32_t)totalPackets[i]);
					observation[i] = (totalReceived[i]/totalPackets[i]);
				}
				double minObservation = 2;
				for (uint8_t i = 0; i<10; i++)
				{
					if (observation[i] < minObservation)
						minObservation = observation[i];
				}
				for (uint8_t i = 0; i<10; i++)
				{
					if (totalPackets[i] != 0)
						m_bandits[f][i]->NewObservation(observation[i]*1.5+.5*minObservation);
					// get new selection
					sf[i] = m_bandits[f][i]->GetSf();
					NS_LOG_DEBUG("[" << (uint32_t)i << "/" << (uint32_t)m_lastChannel << "]: " << std::bitset<5>(sf[i]));
					//m_bandits[f][i]->PrintValues();
				}
				index = 0;
				for (std::list<TableValue>::iterator it = list.begin(); it!=list.end(); it++)
				{
					uint32_t indexVal = 10*index/list.size();
					if (m_settings[it->addr].sfs[f]!=sf[indexVal])
						m_settings[it->addr].acked = false;
					m_settings[it->addr].sfs[f]=sf[indexVal];
					index++;
				}
			}
		}

	uint8_t
		LoRaSfControllerApplication::GetMinDr(uint8_t sfs)
		{
			for (uint8_t i = 0; i<8; i++)
			{
				if ((sfs & 0x80>>i) > 0 )
				{
					return i-2;
				}
			}
			return 255;
		}

	uint8_t
		LoRaSfControllerApplication::GetMaxDr(uint8_t sfs)
		{
			for (uint8_t i = 0; i<8; i++)
			{
				if ((sfs & 1<<i) > 0 )
				{
					return 5-i;
				}
			}
			return 255;
		}

	uint8_t LoRaSfControllerApplication::GetChannelIndexFromFrequency(uint32_t frequency)
	{
		for (uint8_t i = 0; i<3; i++)
			if (m_freqs[i]==frequency)
			{
				return i;
			}
		return 255;
	}

	void 
		LoRaSfControllerApplication::NewPacket (Ptr<const Packet> pkt)
		{
			NS_LOG_FUNCTION(this << pkt);
			LoRaMacHeader header;
			pkt->PeekHeader(header);
			NS_LOG_DEBUG(header.GetAddr ());
			GwTrailer trail;
			pkt->Copy ()->PeekTrailer (trail);
			uint8_t i = GetChannelIndexFromFrequency (trail.GetFrequency());
			m_data[header.GetAddr ()].rssi = trail.GetRssi ();
			m_data[header.GetAddr ()].addr = header.GetAddr ();
			m_data[header.GetAddr ()].received[i]++;
			m_data[header.GetAddr ()].lastPacketNumber = header.GetFrmCounter();
			//NewRssi (trail.GetRssi (), header.GetAddr ());
			std::list<Ptr<LoRaMacCommand>> commands = header.GetCommandList ();
			for (std::list<Ptr<LoRaMacCommand>>::iterator it = commands.begin(); it!=commands.end();++it)
			{
				Ptr<NewChannelAns> ans = DynamicCast<NewChannelAns>(*it);
				if (ans != 0)
					ans->Execute(this,header.GetAddr());
			}
			if (m_network != 0)
			{
				if (!m_settings[header.GetAddr()].acked)
				{
					LoRaMacHeader ans;
					ans.SetAddr (header.GetAddr ());
					ans.SetType (LoRaMacHeader::LORA_MAC_UNCONFIRMED_DATA_DOWN);
					//for (uint8_t f = 0; f<3; f++)
					//{
					//std::tuple<uint16_t, uint8_t,uint8_t> setting = GetSetting (header.GetAddr());
					//std::cout << (uint32_t)m_lastChannel << std::endl;
					if (m_settings[header.GetAddr()].sfs[m_lastChannel] == 0)
					{
						m_settings[header.GetAddr()].acked = true;
						return;
					}
					uint32_t maxSf = std::experimental::randint((int)GetMinDr(m_settings[header.GetAddr()].sfs[m_lastChannel]), (int)GetMaxDr(m_settings[header.GetAddr()].sfs[m_lastChannel]));
					Ptr<NewChannelReq> req = CreateObject<NewChannelReq> (m_lastChannel, m_freqs[m_lastChannel],GetMinDr(m_settings[header.GetAddr()].sfs[m_lastChannel]), maxSf); 
					ans.SetMacCommand(req);
					//}
					Ptr<Packet> ack = Create<Packet>(0);
					ack->AddHeader(ans);
					m_network->Send(ack);
				}
			}
		}

	void 
		LoRaSfControllerApplication::ConfirmDataRate(const Address& address)
		{
			NS_LOG_FUNCTION(this << address);
			//std::cout << "address: " << address << " is acked" << std::endl;
			m_settings[address].acked = true;
		}


} // namespace ns3


