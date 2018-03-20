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

#include "rx-param-setup-ans.h"
#include <ns3/lora-mac-command.h>
#include <ns3/address.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RxParamSetupAns);

RxParamSetupAns::RxParamSetupAns(void)
{
	m_cid = RX_PARAM_SETUP;
	m_direction = TOBASE;
}

RxParamSetupAns::RxParamSetupAns (bool offset, bool dr, bool channel)
{
	m_cid = RX_PARAM_SETUP;
	m_direction = TOBASE;
	m_offsetAck = offset;
	m_drAck = dr;
	m_channelAck = channel;
}

RxParamSetupAns::~RxParamSetupAns (void)
{
}

TypeId
RxParamSetupAns::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RxParamSetupAns")
    .SetParent<LoRaMacCommand> ();
  return tid;
}



TypeId
RxParamSetupAns::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
RxParamSetupAns::GetSerializedSize (void) const
{
  return 2;
}


void
RxParamSetupAns::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_cid);
  i.WriteU8 (0x00|((m_offsetAck<<2)&0x04)|((m_drAck<<1)&0x02)|(m_channelAck&0x01));
}


uint32_t
RxParamSetupAns::Deserialize (Buffer::Iterator start)
{
	start.ReadU8();
	uint8_t temp = start.ReadU8 ();
	m_channelAck = (temp&0x01) == 1;
	m_drAck = (temp&0x02) == 2;
	m_offsetAck = (temp&0x04) == 4;
  return 2;
}

void
RxParamSetupAns::Execute (Ptr<LoRaNetworkApplication> nd,Address address)
{
	std::cout << m_offsetAck << " " << m_drAck << " " << m_channelAck << std::endl;
}
void RxParamSetupAns::SetOffsetAck (bool ack)
{
	m_offsetAck = ack;
}
bool RxParamSetupAns::GetOffsetAck ()
{
	return m_offsetAck;
}
void RxParamSetupAns::SetDrAck (bool ack)
{
	m_drAck = ack;
}
bool RxParamSetupAns::GetDrAck ()
{
	return m_drAck;
}
void RxParamSetupAns::SetChannelAck (bool ack)
{
	m_channelAck = ack;
}
bool RxParamSetupAns::GetChannelAck ()
{
	return m_channelAck;
}

} //namespace ns3

