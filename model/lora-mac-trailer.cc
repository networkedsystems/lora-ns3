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
#include "lora-mac-trailer.h"
#include <ns3/packet.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoRaMacTrailer);

const uint16_t LoRaMacTrailer::LORA_MIC_LENGTH = 4;

LoRaMacTrailer::LoRaMacTrailer (void)
{
}

LoRaMacTrailer::~LoRaMacTrailer (void)
{
}
TypeId
LoRaMacTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaMacTrailer")
    .SetParent<Trailer> ()
    .AddConstructor<LoRaMacTrailer> ()
  ;
  return tid;
}

TypeId
LoRaMacTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
LoRaMacTrailer::Print (std::ostream &os) const
{
  os << " MIC = " << m_MIC;
}

uint32_t
LoRaMacTrailer::GetSerializedSize (void) const
{
  return LORA_MIC_LENGTH;
}

void
LoRaMacTrailer::Serialize (Buffer::Iterator start) const
{
  start.Prev (LORA_MIC_LENGTH);
  start.WriteU32 (m_MIC);
}

uint32_t
LoRaMacTrailer::Deserialize (Buffer::Iterator start)
{
  start.Prev (LORA_MIC_LENGTH);
  m_MIC = start.ReadU32 ();

  return LORA_MIC_LENGTH;
}

uint16_t
LoRaMacTrailer::GetMIC (void) const
{
  return m_MIC;
}

void
LoRaMacTrailer::SetMIC (Ptr<const Packet> p)
{

}

/* Be sure to have removed the trailer and only the trailer
 * from the packet before to use CheckMIC */
bool
LoRaMacTrailer::CheckMIC (Ptr<const Packet> p)
{
  return true;
}

} //namespace ns3
