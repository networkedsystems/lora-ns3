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
#include "lora-network-trailer.h"
#include <ns3/log.h>
#include <ns3/packet.h>
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaNetworkTrailer");
NS_OBJECT_ENSURE_REGISTERED (LoRaNetworkTrailer);


LoRaNetworkTrailer::LoRaNetworkTrailer (void)
{
}

LoRaNetworkTrailer::LoRaNetworkTrailer (uint8_t delay, uint8_t rx1Offset, uint8_t rx2Dr, uint32_t rx2Freq)
{
	m_delay = delay;
	m_rx1Offset = rx1Offset;
	m_rx2Dr = rx2Dr;
	m_rx2Freq = rx2Freq;
}

LoRaNetworkTrailer::~LoRaNetworkTrailer (void)
{
}

TypeId
LoRaNetworkTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaNetworkTrailer")
    .SetParent<Trailer> ()
    .AddConstructor<LoRaNetworkTrailer> ()
  ;
  return tid;
}

TypeId
LoRaNetworkTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
LoRaNetworkTrailer::Print (std::ostream &os) const
{
  os << " RSSI = " << m_delay << ", offset = " << m_rx1Offset << ", data rate = " << m_rx2Dr << ", Frequency = " << m_rx2Freq << std::endl;
}

uint32_t
LoRaNetworkTrailer::GetSerializedSize (void) const
{
  return 7;
}

void
LoRaNetworkTrailer::Serialize (Buffer::Iterator start) const
{
  start.Prev (7);
  start.WriteU8 (m_delay);
  start.WriteU8 (m_rx1Offset);
  start.WriteU8 (m_rx2Dr);
	start.WriteU32 (m_rx2Freq);
}

uint32_t
LoRaNetworkTrailer::Deserialize (Buffer::Iterator start)
{
  start.Prev (7);
	m_delay = start.ReadU8 ();
	m_rx1Offset = start.ReadU8 ();
	m_rx2Dr = start.ReadU8 ();
	m_rx2Freq = start.ReadU32 ();
  return 7;
}
		
uint8_t LoRaNetworkTrailer::GetRx1Offset ()
{
	return m_rx1Offset;
}
void LoRaNetworkTrailer::SetRx1Offset (uint8_t rx1Offset)
{
	m_rx1Offset = rx1Offset;
}
uint32_t LoRaNetworkTrailer::GetRx2Freq ()
{
	return m_rx2Freq;
}
void LoRaNetworkTrailer::SetRx2Freq (uint32_t rx2Freq)
{
	m_rx2Freq = rx2Freq;
}
uint8_t LoRaNetworkTrailer::GetRx2Dr ()
{
	return m_rx2Dr;
}
void LoRaNetworkTrailer::SetRx2Dr (uint8_t rx2Dr)
{
	m_rx2Dr = rx2Dr;
}
uint8_t LoRaNetworkTrailer::GetDelay ()
{
	return m_delay;
}
void LoRaNetworkTrailer::SetDelay (uint8_t delay)
{
	m_delay = delay;
}
} //namespace ns3
