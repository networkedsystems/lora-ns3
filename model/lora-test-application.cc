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
#include "lora-test-application.h"
#include "lora-mac-header.h"
#include "commands/link-check-ans.h"
#include "commands/link-check-req.h"
#include "commands/link-adr-req.h"
#include "commands/link-adr-ans.h"
#include "commands/duty-cycle-req.h"
#include "commands/duty-cycle-ans.h"
#include "commands/rx-param-setup-req.h"
#include "commands/rx-param-setup-ans.h"
#include "commands/dev-status-req.h"
#include "commands/dev-status-ans.h"
#include "commands/new-channel-req.h"
#include "commands/new-channel-ans.h"
#include "commands/rx-timing-setup-req.h"
#include "commands/rx-timing-setup-ans.h"
#include "lora-mac-command.h"
#include "gw-trailer.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaTestApplication");

	NS_OBJECT_ENSURE_REGISTERED (LoRaTestApplication);

	// Application Methods

	TypeId 
		LoRaTestApplication::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaTestApplication")
				.AddConstructor<LoRaTestApplication>()
				.SetParent<LoRaNetworkApplication> ()
				.SetGroupName("LoRa")
				;
			return tid;
		}

	// \brief Application Constructor
	LoRaTestApplication::LoRaTestApplication()
	{
		NS_LOG_FUNCTION (this);
	}

	// \brief LoRaTestApplication Destructor
	LoRaTestApplication::~LoRaTestApplication()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaTestApplication::DoDispose (void)
		{
			NS_LOG_FUNCTION (this);
		}

	void
		LoRaTestApplication::DoInitialize (void)
		{
			Application::DoInitialize ();
		}

	void 
		LoRaTestApplication::NewPacket (Ptr<const Packet> pkt)
		{
			NS_LOG_FUNCTION(this);
			Simulator::Schedule(MilliSeconds(200),&LoRaTestApplication::DelayedNewPacket,this,pkt);
		}

	void
		LoRaTestApplication::DelayedNewPacket(Ptr<const Packet> pkt)
		{
			LoRaMacHeader header;
			pkt->PeekHeader(header);
			NS_LOG_DEBUG(header.GetAddr ());
			GwTrailer trail;
			pkt->Copy ()->PeekTrailer (trail);
			std::list<Ptr<LoRaMacCommand>> commands = header.GetCommandList ();
			for (std::list<Ptr<LoRaMacCommand>>::iterator it = commands.begin(); it!=commands.end();++it)
			{
				(*it)->Execute(this,header.GetAddr());
				//m_network->SetDelayOfDevice(header.GetAddr(),3);
				//m_network->SetSettingsOfDevice(header.GetAddr(),1,5,8681230);

			}
			if (m_network != 0)
			{
				if (header.NeedsAck())
				{
					LoRaMacHeader ans;
					ans.SetAddr (header.GetAddr ());
					ans.SetType (LoRaMacHeader::LORA_MAC_UNCONFIRMED_DATA_DOWN);
					if (m_command != 0)
						ans.SetMacCommand(m_command);	
	//				Ptr<LoRaMacCommand> req = CreateObject<RxParamSetupReq> (1,5,8681230);
//					Ptr<LoRaMacCommand> req = CreateObject<RxTimingSetupReq> (3);
//					Ptr<LoRaMacCommand> req = CreateObject<DutyCycleReq> (0);
//					Ptr<LoRaMacCommand> req = CreateObject<NewChannelReq> (2,8681000,0,0);
//					Ptr<LoRaMacCommand> req = CreateObject<DevStatusReq> ();
				//	ans.SetMacCommand(req);
					m_command = 0;
					Ptr<Packet> ack = Create<Packet>(0);
					ack->AddHeader(ans);
					m_network->Send(ack);
				}
			}
		}

	void LoRaTestApplication::SetMacAnswer (Ptr<LoRaMacCommand> command)
	{
		m_command = command;
	}

} // namespace ns3


