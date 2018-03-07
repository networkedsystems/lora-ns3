/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 The Boeing Company
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

#include "new-channel-req.h"
#include <ns3/lora-mac-command.h>
#include <ns3/new-channel-ans.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NewChannelReq);

NewChannelReq::NewChannelReq(void)
{
	m_cid = NEW_CHANNEL;
	m_direction = FROMBASE;
}

NewChannelReq::NewChannelReq (uint8_t chIndex, uint32_t freq, uint8_t minDr, uint8_t maxDr)
{
	m_cid = NEW_CHANNEL;
	m_direction = FROMBASE;
	m_chIndex = chIndex;
	m_freq = freq;
	m_minDr = minDr;
	m_maxDr = maxDr;
}

NewChannelReq::~NewChannelReq (void)
{
}

TypeId
NewChannelReq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NewChannelReq")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
NewChannelReq::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NewChannelReq::GetSerializedSize (void) const
{
  return 5;
}


void
NewChannelReq::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
	i.WriteU8 (m_chIndex);
	i.WriteU8 ((uint8_t)((m_freq>>16)&0xFF));
	i.WriteU16((uint16_t)(m_freq & 0x0000FFFF));
	uint8_t drRange = ((m_maxDr<<4)&0xF0)|(m_minDr&0x0F);
	i.WriteU8 (drRange);
}


uint32_t
NewChannelReq::Deserialize (Buffer::Iterator start)
{
	m_cid = static_cast<LoRaMacCommandCid>(start.ReadU8 ());
	m_chIndex = start.ReadU8();
	m_freq = (start.ReadU8()<<16)&0x00FF0000 ;
	m_freq |= start.ReadU16()&0x0000FFFF;
	uint8_t drRange = start.ReadU8();
	m_minDr = drRange&0x0F;
	m_maxDr = (drRange >> 4)&0x0F;
  return 5;
}

void
NewChannelReq::Execute (Ptr<LoRaNetDevice> nd,Address address)
{
	bool channel = nd->AddChannel (m_chIndex, m_freq);
	bool dr = nd->SetMaxDataRate(m_maxDr,m_chIndex) && nd->SetMaxDataRate(m_minDr,m_chIndex);
	if (!(channel && dr))
	{
		nd->RemoveChannel (m_chIndex);
	}
	Ptr<LoRaMacCommand> command = CreateObject<NewChannelAns>(dr,channel);
	nd->SetMacAnswer (command);
}

} //namespace ns3

