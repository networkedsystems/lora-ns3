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
#include "lora-no-power-application.h"
#include "lora-mac-header.h"
#include "commands/link-adr-req.h"
#include "commands/link-adr-ans.h"
#include "lora-mac-command.h"
#include "gw-trailer.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaNoPowerApplication");

	NS_OBJECT_ENSURE_REGISTERED (LoRaNoPowerApplication);

	// Application Methods

	TypeId 
		LoRaNoPowerApplication::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaNoPowerApplication")
				.AddConstructor<LoRaNoPowerApplication>()
				.SetParent<LoRaNetworkApplication> ()
				.SetGroupName("LoRa")
				;
			return tid;
		}

	// \brief Application Constructor
	LoRaNoPowerApplication::LoRaNoPowerApplication()
	{
		NS_LOG_FUNCTION (this);
	}

	// \brief LoRaNoPowerApplication Destructor
	LoRaNoPowerApplication::~LoRaNoPowerApplication()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaNoPowerApplication::DoDispose (void)
		{
			NS_LOG_FUNCTION (this);
		}

	void
		LoRaNoPowerApplication::DoInitialize (void)
		{
			Application::DoInitialize ();
		}

	void 
		LoRaNoPowerApplication::NewPacket (Ptr<const Packet> pkt)
		{
			NS_LOG_FUNCTION(this);
			LoRaMacHeader header;
			pkt->PeekHeader(header);
			NS_LOG_DEBUG(header.GetAddr ());
			GwTrailer trail;
			pkt->Copy ()->PeekTrailer (trail);
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
					Ptr<LinkAdrReq> req = CreateObject<LinkAdrReq> (5,1,0xE000,1);
					ans.SetMacCommand(req);
					Ptr<Packet> ack = Create<Packet>(0);
					ack->AddHeader(ans);
					m_network->Send(ack);
				}
			}
		}

} // namespace ns3


