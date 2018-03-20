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
#include "lora-power-application.h"
#include "lora-mac-header.h"
#include "commands/link-adr-req.h"
#include "commands/link-adr-ans.h"
#include "lora-mac-command.h"
#include "gw-trailer.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaPowerApplication");

	NS_OBJECT_ENSURE_REGISTERED (LoRaPowerApplication);

	// Application Methods

	TypeId 
		LoRaPowerApplication::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaPowerApplication")
				.AddConstructor<LoRaPowerApplication>()
				.SetParent<LoRaNetworkApplication> ()
				.SetGroupName("LoRa")
				;
			return tid;
		}

	// \brief Application Constructor
	LoRaPowerApplication::LoRaPowerApplication()
	{
		NS_LOG_FUNCTION (this);
		Simulator::Schedule(Seconds(600),&LoRaPowerApplication::CalculateSetting,this);
	}

	// \brief LoRaPowerApplication Destructor
	LoRaPowerApplication::~LoRaPowerApplication()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaPowerApplication::DoDispose (void)
		{
			NS_LOG_FUNCTION (this);
			m_RSSI.clear();
			m_settings.clear();
		}

	void
		LoRaPowerApplication::DoInitialize (void)
		{
			Application::DoInitialize ();
		}

	// Protected methods
	// StartApp and StopApp will likely be overridden by application subclasses
	void LoRaPowerApplication::StartApplication ()
	{ // Provide null functionality in case subclass is not interested
		NS_LOG_FUNCTION (this);

	}

	void LoRaPowerApplication::StopApplication ()
	{ // Provide null functionality in case subclass is not interested
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaPowerApplication::NewRssi (double rssi, const Address& address)
		{
			NS_LOG_FUNCTION(this << rssi << address);
			for (auto &it : m_RSSI)
			{
				if (address == std::get<0>(it))
				{
					if (std::get<1>(it)!=-200)
					{
						std::get<1>(it) = std::get<1>(it)*.95+.05*(10*std::log10(rssi*125000)-(5-std::get<2>(it))*3+2);
					}
					else
					{
						std::get<1>(it) = 10*std::log10(rssi*125000)+2;
					}
					break;

				}
			}
		}


	std::tuple<uint16_t, uint8_t,uint8_t>
		LoRaPowerApplication::GetSetting (const Address& address)
		{
			return m_settings[address];
			//for (auto &jt : m_settings)
			//{
			//	if (address == std::get<0>(jt))
			//	{
			//		std::cout << "datarate " << (uint32_t)std::get<2>(jt) << " and power  " << (uint32_t)std::get<3>(jt) << std::endl;
			//		return std::make_tuple(std::get<1>(jt),std::get<2>(jt),std::get<3>(jt));
			//	}
			//}
			//return std::make_tuple(0,0,0);
		}

	void
		LoRaPowerApplication::SaveSetting (const Address& address, uint8_t power, uint8_t datarate, uint16_t channelMask)
		{
			m_settings[address] = std::make_tuple(channelMask, datarate,power);
		//for (auto &jt : m_settings)
		//{
		//	if (address == std::get<0>(jt))
		//	{
		//		std::get<1>(jt) = channelMask; 
		//		std::get<2>(jt) = datarate;
		//		std::get<3>(jt) = power;
		//		break;
		//	}
		//}
		}
	void
		LoRaPowerApplication::ConfirmPower (const Address& address)
		{
			NS_LOG_FUNCTION (this << address);
			for (auto &it : m_RSSI)
			{
				if (address == std::get<0>(it))
				{
					std::get<2>(it) = std::get<2>(m_settings[address]);
				//for (auto &jt : m_settings)
				//{
				//	if (address == std::get<0>(jt))
				//	{
				//		std::get<2>(it) = std::get<3>(jt);
				//		break;
				//	}
				//}
				}
			}
		}

	bool
		LoRaPowerApplication::CheckTuple(const std::tuple<Address,double,uint8_t,uint8_t,uint16_t>& first,const std::tuple<Address,double,uint8_t,uint8_t,uint16_t>& second)
		{
			return std::round(std::get<1>(first)) > std::round(std::get<1>(second));
		}



	void
		LoRaPowerApplication::CalculateSetting (void)
		{
			Simulator::Schedule(Seconds(60*15*10),&LoRaPowerApplication::CalculateSetting,this);
			//SORT
			m_RSSI.sort(LoRaPowerApplication::CheckTuple);
			//Select 1/3 
			for (uint8_t groupIndex = 1; groupIndex < 4; groupIndex++)
			{
				std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator listStart = m_RSSI.begin();
				std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator listEnd = m_RSSI.begin();
				if (groupIndex == 1)
				{
					std::advance(listEnd,std::ceil((double)m_RSSI.size()/3.0));
				}
				else if (groupIndex == 2)
				{
					std::advance(listStart,std::ceil((double)m_RSSI.size()/3.0));
					std::advance(listEnd,std::ceil((double)m_RSSI.size()*2.0/3.0));
				}
				else
				{
					std::advance(listStart,std::ceil((double)m_RSSI.size()*2.0/3.0));
					listEnd = m_RSSI.end();
				}
				CalculatePower (groupIndex,listStart,listEnd);
			}
		}

	void
		LoRaPowerApplication::CalculatePower (uint8_t groupIndex, std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator start, std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator end)
		{
			// Get spreading factor 

			// with other spreading factors look at highest RSSI
			bool found = false;
			std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator next = start;
			uint8_t powerSettingSecond = 5;
			uint8_t powerSetting = 5;
			uint16_t channelMask = 2<<(15-groupIndex);

			std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator second = GetSpreading(start,end);

			for (std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator listIt = start; listIt!=end; ++listIt)
			{
				second = GetSpreading(listIt,end);
				if (m_RSSI.size() > 9) 
				{
					for (uint8_t powerj = 5; powerj>0; powerj--)
					{
						if(std::get<1>(*listIt) < 9.5+(5-powerj)*3+std::get<1>(*second))
						{
							powerSettingSecond = powerj;
							found = true;
							break;
						}
						std::cout << powerj << std::endl;
					}
					std::cout << std::get<1>(*listIt) << " < 9.5+" << (uint32_t)(5-powerSettingSecond) << "*3 " << std::get<1>(*second) << std::endl; 
					next = listIt;
					if(found)
						break;
					SaveSetting(std::get<0>(*listIt),5,5,channelMask);
					std::get<3>(*listIt) = 5;
					std::get<4>(*listIt) = channelMask;
					std::cout << (uint32_t)channelMask << " " << (uint32_t)groupIndex << "  " << Mac32Address::ConvertFrom(std::get<0>(*listIt)).GetUInt() << " 5 2 " << (uint32_t)std::get<2>(*listIt) << " " << std::get<1>(*listIt)<< " " << Simulator::Now().GetSeconds()<< std::endl;
				}
				else
				{
					break;
				}
			}
			std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator first = next;

			for (std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator listIt = next; listIt!=end; ++listIt)
			{
				//apply power control
				powerSetting = 5;
				uint8_t sfSetting = std::get<3>(*listIt);
				if (m_RSSI.size() > 9) 
				{
					if (sfSetting == 5)
					{
						for (uint8_t powerj = 5; powerj>0; powerj--)
						{
							if((5-powerj)*3+std::get<1>(*listIt)+7  > (5-powerSettingSecond)*3+std::get<1>(*second)+2)
							{
								powerSetting = powerj;
								break;
							}
						}
					}
					else
					{
						for (uint8_t poweri = 5; poweri>0; poweri--){
							if((5-poweri)*3+std::get<1>(*listIt)+19.5-2.5*sfSetting  >std::get<1>(*first))
							{
								powerSetting = poweri;
								break;
							}
						}

					}
				}
				std::cout << (uint32_t)channelMask << " " << (uint32_t)groupIndex << "  " << Mac32Address::ConvertFrom(std::get<0>(*listIt)).GetUInt() << " " <<  (uint32_t) sfSetting << " " << (uint32_t)powerSetting << " " << (uint32_t)std::get<2>(*listIt) << " " << std::get<1>(*listIt)<< " " << Simulator::Now().GetSeconds()<< std::endl;
				//first 2 should be within reach of power control
				if (sfSetting>5)
				{
					std::cout << "ERROR spreading" <<std::endl;
					sfSetting = 5;
				}
				if (powerSetting>5)
				{
					std::cout << "ERROR power" <<std::endl;
					powerSetting = 5;
				}
				if ((channelMask != (2<<13)) && (channelMask != (2<<12)) && (channelMask!= (2<<14)))
				{
					std::cout << "ERROR CHANNELMASK " << channelMask << " " << (2<<12) << " " << (2<<13) << " " << (2<<14) <<std::endl;
					channelMask = 2<<14;
				}
				if (powerSetting == 0)
				{
					std::cout << "ERROR power0" <<std::endl;
					powerSetting = 1;
				}
				SaveSetting(std::get<0>(*listIt),powerSetting,sfSetting,channelMask);
			}
		}

    std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator
       LoRaPowerApplication::GetSpreading(std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator start, std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator end)
       {
           int dist = std::distance(start,end);
           int index = 0;
           //First Rb/sumRb = 1 and so on and so on
           double sumRb = 0;
           for (double SF = 7; SF< 13; SF++)
               sumRb += SF/std::pow(2,SF);
           uint8_t sfSetting = 5;
           double sumRbi = 0;
           uint8_t spreading = 7;
           sumRbi += spreading/std::pow(2,spreading);
           std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator second;
           for(start = start; start != end; ++start)
           {
               std::get<3>(*start) = sfSetting;
               if ((double)index > (double)dist*(sumRbi/sumRb))
               {
                   if(spreading == 8)
                       second = start;
                   spreading++;
                   sfSetting--;
                   sumRbi += spreading/std::pow(2,spreading);
               }
               index++;
           }
           return second;
       }

	void 
		LoRaPowerApplication::NewPacket (Ptr<const Packet> pkt)
		{
			NS_LOG_FUNCTION(this);
			LoRaMacHeader header;
			pkt->PeekHeader(header);
			NS_LOG_DEBUG(header.GetAddr ());
			GwTrailer trail;
			pkt->Copy ()->PeekTrailer (trail);
			NewRssi (trail.GetRssi (), header.GetAddr ());
			std::list<Ptr<LoRaMacCommand>> commands = header.GetCommandList ();
			for (std::list<Ptr<LoRaMacCommand>>::iterator it = commands.begin(); it!=commands.end();++it)
			{
				Ptr<LinkAdrAns> ans = DynamicCast<LinkAdrAns>(*it);
				if (ans != 0)
					ans->Execute(this,header.GetAddr());
			}
			if (m_network != 0)
			{
				if (header.NeedsAck())
				{
					LoRaMacHeader ans;
					ans.SetAddr (header.GetAddr ());
					ans.SetType (LoRaMacHeader::LORA_MAC_UNCONFIRMED_DATA_DOWN);
					std::tuple<uint16_t, uint8_t,uint8_t> setting = GetSetting (header.GetAddr());
					Ptr<LinkAdrReq> req = CreateObject<LinkAdrReq> (std::get<0>(setting),std::get<1>(setting),std::get<2>(setting),1);
					ans.SetMacCommand(req);
					Ptr<Packet> ack = Create<Packet>(0);
					ack->AddHeader(ans);
					m_network->Send(ack);
				}
			}
		}

} // namespace ns3


