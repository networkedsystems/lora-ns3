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
#include "link-adr-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>
#include <ns3/log.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LinkAdrAns);
NS_LOG_COMPONENT_DEFINE ("LinkAdrAns");

LinkAdrAns::LinkAdrAns(void)
{
	m_cid = LINK_ADR;
	m_direction = TOBASE;
	m_power = false;
	m_datarate = false;
	m_channelmask = false;
}

LinkAdrAns::LinkAdrAns (bool power, bool datarate, bool channelmask)
{
	m_cid = LINK_ADR;
	m_direction = TOBASE;
	m_power = power;
	m_datarate = datarate;
	m_channelmask = channelmask;
}

LinkAdrAns::~LinkAdrAns (void)
{
}

TypeId
LinkAdrAns::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LinkAdrAns")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
LinkAdrAns::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LinkAdrAns::GetSerializedSize (void) const
{
  return 2;
}


void
LinkAdrAns::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
	i.WriteU8 (0x00|((m_power&0x1)<<2)|((m_datarate&0x1)<<1)|(m_channelmask&0x1));
}


uint32_t
LinkAdrAns::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_cid = static_cast<LoRaMacCommandCid>(i.ReadU8 ());
	uint8_t temp = i.ReadU8 ();
	m_power = (temp >> 2)&0x1;
	m_datarate = (temp >> 1) & 0x1;
	m_channelmask = (temp) & 0x1;
  return 2;
}

void
LinkAdrAns::Execute (Ptr<LoRaNetworkApplication> app, Address address)
{
	if (app !=0)
	{
		if (m_power)
		{
			app->ConfirmPower (address);
		}
		if (m_datarate)
			app->ConfirmDataRate (address);
		if (m_channelmask)
			app->ConfirmChannelMask (address);
	}
}

bool
LinkAdrAns::PowerSet (void)
{
	return m_power;
}

bool 
LinkAdrAns::DatarateSet (void)
{
	return m_datarate;
}

bool 
LinkAdrAns::ChannelMaskSet (void)
{
	return m_channelmask;
}

void
LinkAdrAns::SetPower (bool power)
{
	m_power = power;
}

void 
LinkAdrAns::SetDatarate (bool datarate)
{
	m_datarate = datarate;
}

void
LinkAdrAns::SetChannelMask (bool channelmask)
{
	m_channelmask = channelmask;
}

} //namespace ns3

