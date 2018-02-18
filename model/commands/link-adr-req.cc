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

#include "link-adr-req.h"
#include <ns3/lora-mac-command.h>
#include <ns3/link-adr-ans.h>
#include <ns3/address.h>

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (LinkAdrReq);

LinkAdrReq::LinkAdrReq(void)
{
	
	m_cid = LINK_ADR;
	m_direction = FROMBASE;
	m_dataRate = 0;
	m_power = 1;
	m_channelMask = 0xE000;
	m_nbRep = 1;
}

LinkAdrReq::LinkAdrReq (uint8_t dataRate, uint8_t power, uint16_t channelMask, uint8_t repetitions)
{
	m_cid = LINK_ADR;
	m_direction = FROMBASE;
	m_dataRate = dataRate;
	m_power = power;
	m_channelMask = channelMask;
	m_nbRep = repetitions;
}	

LinkAdrReq::~LinkAdrReq (void)
{
}

TypeId
LinkAdrReq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LinkAdrReq")
    .SetParent<LoRaMacCommand> ();
  return tid;
}

TypeId
LinkAdrReq::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LinkAdrReq::GetSerializedSize (void) const
{
  return 5;
}


void
LinkAdrReq::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
	i.WriteU8 ((m_dataRate&0x7) << 4 | (m_power&0x7));
	i.WriteU16 (m_channelMask);
	i.WriteU8 ( (m_nbRep && 0x7) | (0) << 4); 
}


uint32_t
LinkAdrReq::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
	m_cid = static_cast<LoRaMacCommandCid>(i.ReadU8 ());
	uint8_t temp = i.ReadU8 ();
	m_power = temp & 0x7;
	m_dataRate = (temp >> 4) & 0x7;
	m_channelMask = i.ReadU16 ();
	m_nbRep = i.ReadU8() && 0x7;
  return 5;
}

void
LinkAdrReq::Execute (Ptr<LoRaNetDevice> netDevice,Address address)
{
	bool datarate = netDevice->SetMaxDataRate (m_dataRate);
	bool power = netDevice->SetMaxPower (m_power);
	bool channelmask = netDevice->SetChannelMask (m_channelMask);
	netDevice->SetNbRep(m_nbRep);
	LinkAdrAns* command = new LinkAdrAns ( datarate, power, channelmask);
	netDevice->SetMacAnswer (command);
}

void
LinkAdrReq::SetDataRate (uint8_t dataRate)
{
	m_dataRate = dataRate;
}

void
LinkAdrReq::SetPower (uint8_t power)
{
	m_power = power;
}

void 
LinkAdrReq::SetChannelMask (uint16_t channelMask)
{
	m_channelMask = channelMask;
}

void
LinkAdrReq::SetRepetitions (uint8_t repetitions)
{
	m_nbRep = repetitions;
}

uint8_t 
LinkAdrReq::GetDataRate (void)
{
	return m_dataRate;
}

uint8_t 
LinkAdrReq::GetPower (void)
{
	return m_power;
}

uint16_t
LinkAdrReq::GetChannelMask (void)
{
	return m_channelMask;
}

uint8_t 
LinkAdrReq::GetRepetitions (void)
{
	return m_nbRep;
}

} //namespace ns3

