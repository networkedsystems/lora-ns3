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

#include "rx-param-setup-req.h"
#include "rx-param-setup-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RxParamSetupReq);

RxParamSetupReq::RxParamSetupReq(void)
{
	
	m_cid = RX_PARAM_SETUP;
	m_direction = FROMBASE;
}

RxParamSetupReq::RxParamSetupReq (uint8_t rx1Offset, uint8_t rx2Dr, uint32_t rx2Freq)
{
	m_cid = RX_PARAM_SETUP;
	m_direction = FROMBASE;
	m_rx1Offset = rx1Offset;
	m_rx2Dr = rx2Dr;
	m_rx2Freq = rx2Freq;
}

RxParamSetupReq::~RxParamSetupReq (void)
{
}

TypeId
RxParamSetupReq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RxParamSetupReq")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
RxParamSetupReq::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
RxParamSetupReq::GetSerializedSize (void) const
{
  return 5;
}


void
RxParamSetupReq::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
	uint8_t DLSettings = m_rx2Dr;
	DLSettings |= (m_rx1Offset << 4)&0x70;
	i.WriteU8 (DLSettings);
	i.WriteU8 ((uint8_t)((m_rx2Freq >> 16)&0x000000ff));
	i.WriteU8 ((uint8_t)((m_rx2Freq >> 8)&0x000000ff));
	i.WriteU8 ((uint8_t)((m_rx2Freq)&0x000000ff));
}


uint32_t
RxParamSetupReq::Deserialize (Buffer::Iterator start)
{
	//read cid Put assert here. 
	start.ReadU8 ();
	uint8_t DLSettings = start.ReadU8 ();
	m_rx2Dr = DLSettings&0x0F;
	m_rx1Offset = (DLSettings >> 4)&0x07;
	m_rx2Freq = (start.ReadU8 ()<<16) & 0x00ff0000;
	m_rx2Freq |= (start.ReadU8 ()<<8) & 0x0000ff00;
	m_rx2Freq |= (start.ReadU8 ()) & 0x000000ff;
  return 5;
}

void
RxParamSetupReq::Execute (Ptr<LoRaNetDevice> nd,Address address)
{
	bool offset =	nd->SetDlOffset (m_rx1Offset);
	bool settings = nd->SetRx2Settings (m_rx2Dr, m_rx2Freq);
	Ptr<LoRaMacCommand> command = CreateObject<RxParamSetupAns>(offset,settings,settings);
	nd->SetMacAnswer (command);
}


uint8_t RxParamSetupReq::GetRx1Offset ()
{
	return m_rx1Offset;
}
void RxParamSetupReq::SetRx1Offset (uint8_t rx1Offset)
{
	m_rx1Offset = rx1Offset;
}
uint32_t RxParamSetupReq::GetRx2Freq ()
{
	return m_rx2Freq;
}
void RxParamSetupReq::SetRx2Freq (uint32_t rx2Freq)
{
	m_rx2Freq = rx2Freq;
}
uint8_t RxParamSetupReq::GetRx2Dr ()
{
	return m_rx2Dr;
}
void RxParamSetupReq::SetRx2Dr (uint8_t rx2Dr)
{
	m_rx2Dr = rx2Dr;
}

} //namespace ns3

