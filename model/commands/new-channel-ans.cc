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

#include "new-channel-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NewChannelAns);

NewChannelAns::NewChannelAns(bool datarateOk, bool freqOk)
{
	m_cid = NEW_CHANNEL;
	m_direction = TOBASE;
	m_freqOk = freqOk;
	m_datarateOk = datarateOk;
}

NewChannelAns::NewChannelAns(void)
{
	m_cid = NEW_CHANNEL;
	m_direction = TOBASE;
	m_freqOk = true;
	m_datarateOk = true;
}

NewChannelAns::~NewChannelAns (void)
{
}

TypeId
NewChannelAns::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NewChannelAns")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
NewChannelAns::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NewChannelAns::GetSerializedSize (void) const
{
  return 2;
}


void
NewChannelAns::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
	uint8_t temp = 0;
	if (m_freqOk)
		temp = 1;
	else
		temp = 0;
	if (m_datarateOk)
		temp ^= 2;
	else
		temp ^= 0;
	i.WriteU8(temp);
}


uint32_t
NewChannelAns::Deserialize (Buffer::Iterator start)
{
	m_cid = static_cast<LoRaMacCommandCid>(start.ReadU8 ());
	uint8_t temp = start.ReadU8();
	m_freqOk = ((temp&0x01) == 1);
	m_datarateOk = ((temp&0x02) == 2);
  return 2;
}

void
NewChannelAns::Execute (Ptr<LoRaNetworkApplication> nd,Address address)
{
	std::cout << m_freqOk << " " << m_datarateOk << std::endl;
}
	
void 
NewChannelAns::SetDatarateOk (bool drOk)
{
	m_datarateOk = drOk;
}
bool 
NewChannelAns::GetDatarateOk ()
{
	return m_datarateOk;
}
void 
NewChannelAns::SetFreqOk (bool freqOk)
{
	m_freqOk = freqOk;
}

bool
NewChannelAns::GetFreqOk (void)
{
	return m_freqOk;
}

} //namespace ns3

