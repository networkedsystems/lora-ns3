/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 *  Author: Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#include "link-check-req.h"
#include "link-check-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LinkCheckReq);

LinkCheckReq::LinkCheckReq(void)
{
	
	m_cid = LINK_CHECK;
	m_direction = TOBASE;
}

LinkCheckReq::~LinkCheckReq (void)
{
}

//std::string
//LinkCheckReq::GetName (void) const
//{
//  return "LoRa MAC Command";/
//}

TypeId
LinkCheckReq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LinkCheckReq")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
LinkCheckReq::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LinkCheckReq::GetSerializedSize (void) const
{
  return 1;
}


void
LinkCheckReq::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
}


uint32_t
LinkCheckReq::Deserialize (Buffer::Iterator start)
{
	start.ReadU8 ();
  return 1;
}

void
LinkCheckReq::Execute(Ptr<LoRaNetworkApplication> app, Address address)
{
	if (app!=0)
	{
		uint8_t margin = app->GetNetwork()->GetMargin(address);
		uint8_t count = app->GetNetwork()->GetCount(address);
		Ptr<LoRaMacCommand> command = CreateObject<LinkCheckAns>(margin,count);
		app->SetMacAnswer (command);
	}
}

} //namespace ns3

