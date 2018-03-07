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

#include "dev-status-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DevStatusAns);

DevStatusAns::DevStatusAns(void)
{
	m_cid = DEV_STATUS;
	m_direction = TOBASE;
}

DevStatusAns::DevStatusAns(uint8_t battery, int8_t margin)
{
	m_cid = DEV_STATUS;
	m_direction = TOBASE;
	m_battery = battery;
	m_margin = margin;
}

DevStatusAns::~DevStatusAns (void)
{
}

TypeId
DevStatusAns::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DevStatusAns")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
DevStatusAns::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DevStatusAns::GetSerializedSize (void) const
{
  return 3;
}


void
DevStatusAns::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
	i.WriteU8 (m_battery);
	union X  
	{  
	    int8_t intValue;  
	    uint8_t uintValue;  
	} marginConversion; 
	marginConversion.intValue = m_margin;
	i.WriteU8 (marginConversion.uintValue);
}


uint32_t
DevStatusAns::Deserialize (Buffer::Iterator start)
{
	union X  
	{  
	    int8_t intValue;  
	    uint8_t uintValue;  
	} marginConversion;  
	start.ReadU8();
	m_battery = start.ReadU8();
	marginConversion.uintValue = start.ReadU8();
	m_margin = marginConversion.intValue;
  return 3;
}

void
DevStatusAns::Execute (Ptr<LoRaNetworkApplication> app,Address address)
{
	std::cout << (int32_t)m_margin << " " << (uint32_t)m_battery << std::endl;
	//nd->GetSNR();
	//Ptr<LoRaMacCommand> command = CreateObject<LinkCheckAns>(margin,count);
	//nd->SetMacAnswer (command);
}

void
DevStatusAns::SetBattery (uint8_t battery)
{
	m_battery = battery;
}

uint8_t 
DevStatusAns::GetBattery ()
{
	return m_battery;
}

void
DevStatusAns::SetMargin (int8_t margin)
{
	m_margin = margin;
}

int8_t 
DevStatusAns::GetMargin ()
{
	return m_margin;
}
} //namespace ns3

