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
 * Author: Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#include "link-check-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LinkCheckAns);

LinkCheckAns::LinkCheckAns(void)
{
	m_cid = LINK_CHECK;
	m_direction = FROMBASE;
	m_margin = 0;
	m_count = 1;
}

LinkCheckAns::LinkCheckAns(uint8_t margin, uint8_t count)
{
	m_cid = LINK_CHECK;
	m_direction = FROMBASE;
	m_margin = margin;
	m_count = count;
}

LinkCheckAns::~LinkCheckAns (void)
{
}

TypeId
LinkCheckAns::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LinkCheckAns")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
LinkCheckAns::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LinkCheckAns::GetSerializedSize (void) const
{
  return 3;
}


void
LinkCheckAns::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
	i.WriteU8 (m_margin);
	i.WriteU8 (m_count);
}


uint32_t
LinkCheckAns::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
	m_cid = static_cast<LoRaMacCommandCid>(i.ReadU8 ());
	m_margin = i.ReadU8();
	m_count = i.ReadU8();
  return 3;
}

void
LinkCheckAns::Execute (Ptr<LoRaNetDevice> nd,Address address)
{
	// Do Nothing, probably, pass this to a higher layer, or use this to control the power.
	std::cout << (uint32_t)m_margin << " " << (uint32_t)m_count << std::endl;
}

void LinkCheckAns::SetMargin (uint8_t margin)
{
	m_margin = margin;
}
uint8_t 
LinkCheckAns::GetMargin (void)
{
	return m_margin;
}
void LinkCheckAns::SetCount (uint8_t count)
{
	m_count = count;
}

uint8_t 
LinkCheckAns::GetCount (void)
{
	return m_count;
}

} //namespace ns3

