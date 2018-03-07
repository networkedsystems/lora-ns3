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

#include "duty-cycle-req.h"
#include "duty-cycle-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DutyCycleReq);

DutyCycleReq::DutyCycleReq(void)
{
	m_cid = DUTY_CYCLE;
	m_direction = FROMBASE;
}

DutyCycleReq::DutyCycleReq(uint8_t dutyCycle)
{
	m_cid = DUTY_CYCLE;
	m_direction = FROMBASE;
	m_dutyCycle = dutyCycle;
}

DutyCycleReq::~DutyCycleReq (void)
{
}

TypeId
DutyCycleReq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DutyCycleReq")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
DutyCycleReq::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DutyCycleReq::GetSerializedSize (void) const
{
  return 2;
}


void
DutyCycleReq::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
	i.WriteU8(m_dutyCycle);
}


uint32_t
DutyCycleReq::Deserialize (Buffer::Iterator start)
{
	m_cid = static_cast<LoRaMacCommandCid>(start.ReadU8 ());
	m_dutyCycle = start.ReadU8();
  return 2;
}

void
DutyCycleReq::Execute (Ptr<LoRaNetDevice> nd,Address address)
{
	nd->SetDutyCycle(m_dutyCycle);
	Ptr<LoRaMacCommand> command = CreateObject<DutyCycleAns>();
	nd->SetMacAnswer (command);
}

void
DutyCycleReq::SetDutyCycle (uint8_t dutyCycle)
{
	m_dutyCycle = dutyCycle;
}

uint8_t 
DutyCycleReq::GetDutyCycle ()
{
	return m_dutyCycle;
}

} //namespace ns3

