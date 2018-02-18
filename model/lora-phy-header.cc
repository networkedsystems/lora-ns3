/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
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

#include <ns3/log.h>
#include "lora-phy-header.h"
#include <ns3/buffer.h>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaPhyHeader");

NS_OBJECT_ENSURE_REGISTERED (LoRaPhyHeader);

TypeId
LoRaPhyHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaPhyHeader")
    .SetParent<Header> ()
    .SetGroupName ("LoRa")
    .AddConstructor<LoRaPhyHeader> ()
  ;
  return tid;
}

LoRaPhyHeader::LoRaPhyHeader ()
{
  m_preamble = 8;
  m_beacon=false;
}

LoRaPhyHeader::~LoRaPhyHeader ()
{
}

TypeId
LoRaPhyHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
LoRaPhyHeader::SetPreamble (uint16_t preamble)
{
  if (preamble > 8)
  {
    m_preamble = preamble;
  }
}

uint16_t 
LoRaPhyHeader::GetPreamble () const
{
  return m_preamble;
}

void
LoRaPhyHeader::SetBeacon ()
{
  m_beacon = true;
}

void
LoRaPhyHeader::SetData()
{
  m_beacon = false;
}

bool
LoRaPhyHeader::IsBeacon ()
{
  return m_beacon;
}

bool
LoRaPhyHeader::IsData()
{
  return !m_beacon;
}

uint32_t
LoRaPhyHeader::GetSerializedSize (void) const
{
	//beacons do not have PHYheader information, only preamble
  if (m_beacon){
    return m_preamble;
  }
  else
  {
     return 2+m_preamble;
  }
}



void
LoRaPhyHeader::Serialize (Buffer::Iterator start) const
{
	//write random data, because no information about this field.
  uint8_t data = 8;
  for (uint16_t i = 0; i<m_preamble;i++)
  {
    start.WriteU8(data);
  }
  if (!m_beacon)
  {
    start.WriteU8(data);
    start.WriteU8(data);
  }
}

uint32_t
LoRaPhyHeader::Deserialize (Buffer::Iterator start)
{
  for (uint16_t i = 0; i<m_preamble;i++)
  {
    start.ReadU8();
  }
  if(!m_beacon)
  {
    start.ReadU8();
    start.ReadU8();
  }
  return GetSerializedSize ();
}


void
LoRaPhyHeader::Print (std::ostream &os) const
{

  os << "PHYHEADER";
}



} // namespace ns3


