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

#include "rx-timing-setup-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RxTimingSetupAns);

RxTimingSetupAns::RxTimingSetupAns(void)
{
	m_cid = RX_TIMING;
	m_direction = TOBASE;
}

RxTimingSetupAns::~RxTimingSetupAns (void)
{
}

TypeId
RxTimingSetupAns::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RxTimingSetupAns")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
RxTimingSetupAns::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
RxTimingSetupAns::GetSerializedSize (void) const
{
  return 1;
}


void
RxTimingSetupAns::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
}


uint32_t
RxTimingSetupAns::Deserialize (Buffer::Iterator start)
{
	start.ReadU8();
  return 1;
}

void
RxTimingSetupAns::Execute (Ptr<LoRaNetworkApplication> nd,Address address)
{
	//std::cout << "Acked" << std::endl;
}

} //namespace ns3

