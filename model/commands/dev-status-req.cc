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

#include "dev-status-req.h"
#include "dev-status-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/lora-phy.h>
#include <ns3/address.h>
#include <ns3/energy-source.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DevStatusReq);

DevStatusReq::DevStatusReq(void)
{
	
	m_cid = DEV_STATUS;
	m_direction = FROMBASE;
}

DevStatusReq::~DevStatusReq (void)
{
}

TypeId
DevStatusReq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DevStatusReq")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
DevStatusReq::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DevStatusReq::GetSerializedSize (void) const
{
  return 1;
}


void
DevStatusReq::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
}


uint32_t
DevStatusReq::Deserialize (Buffer::Iterator start)
{
	start.ReadU8();
  return 1;
}

void
DevStatusReq::Execute (Ptr<LoRaNetDevice> nd,Address address)
{
	double tempmargin = nd->GetPhy()->GetSnrLastPacket();
	int8_t margin = tempmargin;
	if (tempmargin > 32)
		margin = 32;
	else if (tempmargin < -32)
		margin = -32;
	// initialize with 0, this means connected with unlimited energy source.
	uint8_t battery = 0;
	if (nd->GetNode()->GetObject<EnergySource>())
		battery = (nd->GetNode()->GetObject<EnergySource>()->GetEnergyFraction()*254)+1;
	Ptr<LoRaMacCommand> command = CreateObject<DevStatusAns>(battery,margin);
	nd->SetMacAnswer (command);
}

} //namespace ns3

