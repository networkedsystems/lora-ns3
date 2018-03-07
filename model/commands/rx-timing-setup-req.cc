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

#include "rx-timing-setup-req.h"
#include <ns3/lora-mac-command.h>
#include <ns3/rx-timing-setup-ans.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RxTimingSetupReq);

RxTimingSetupReq::RxTimingSetupReq (void)
{
	m_cid = RX_TIMING;
	m_direction = TOBASE;
	m_delay = 0;
}

RxTimingSetupReq::RxTimingSetupReq (uint8_t del)
{
	m_cid = RX_TIMING;
	m_direction = TOBASE;
	m_delay = del;
}

RxTimingSetupReq::~RxTimingSetupReq (void)
{
}

TypeId
RxTimingSetupReq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RxTimingSetupReq")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
RxTimingSetupReq::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
RxTimingSetupReq::GetSerializedSize (void) const
{
  return 2;
}


void
RxTimingSetupReq::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
  i.WriteU8 (m_delay);
}


uint32_t
RxTimingSetupReq::Deserialize (Buffer::Iterator start)
{
	m_cid = static_cast<LoRaMacCommandCid>(start.ReadU8 ());
	m_delay = start.ReadU8();
  return 2;
}

void
RxTimingSetupReq::Execute (Ptr<LoRaNetDevice> nd,Address address)
{
	nd->SetDelay(m_delay);
	Ptr<LoRaMacCommand> command = CreateObject<RxTimingSetupAns>();
	nd->SetMacAnswer (command);
}

uint8_t 
RxTimingSetupReq::GetDelay (void)
{
	return m_delay;
}

void
RxTimingSetupReq::SetDelay (uint8_t delay)
{
	NS_ASSERT(delay < 0x10);
	m_delay = delay;
}

} //namespace ns3

