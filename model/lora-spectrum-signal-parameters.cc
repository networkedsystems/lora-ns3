/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 The Boeing Company
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
#include "lora-spectrum-signal-parameters.h"
#include <ns3/log.h>
#include <ns3/packet.h>
#include "ns3/antenna-model.h"
#include "ns3/spectrum-phy.h"
#include "ns3/spectrum-value.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaSpectrumSignalParameters");

LoRaSpectrumSignalParameters::LoRaSpectrumSignalParameters (void)
{
  NS_LOG_FUNCTION (this);
}

LoRaSpectrumSignalParameters::LoRaSpectrumSignalParameters (const LoRaSpectrumSignalParameters& p)
  : SpectrumSignalParameters (p)
{
  NS_LOG_FUNCTION (this << &p);
  packet = p.packet->Copy ();
  m_channel = p.m_channel;
  m_bandwidth = p.m_bandwidth;
  m_spreading = p.m_spreading;
  m_ber = 0;
}

LoRaSpectrumSignalParameters::~LoRaSpectrumSignalParameters()
{
	packet = 0;
	if (txAntenna !=0)
		txAntenna = 0;
	if (txPhy != 0)
		txPhy = 0;
	if (psd !=0)
		psd = 0;

}

Ptr<SpectrumSignalParameters>
LoRaSpectrumSignalParameters::Copy (void)
{
  NS_LOG_FUNCTION (this);
  return Create<LoRaSpectrumSignalParameters> (*this);
}

uint32_t
LoRaSpectrumSignalParameters::GetChannel ()
{
  return m_channel;
}

void
LoRaSpectrumSignalParameters::SetChannel (uint32_t channel)
{
  m_channel = channel;
}

void 
LoRaSpectrumSignalParameters::SetBandwidth (uint32_t bw)
{
	m_bandwidth = bw;
}

uint32_t
LoRaSpectrumSignalParameters::GetBandwidth ()
{
  return m_bandwidth;
}

void 
LoRaSpectrumSignalParameters::SetBer (uint32_t ber)
{
  m_ber = ber;
}

uint16_t
LoRaSpectrumSignalParameters::GetBer ()
{
  return m_ber;
}

void 
LoRaSpectrumSignalParameters::SetSpreading (uint16_t spreading)
{
  m_spreading = spreading;
}

uint16_t
LoRaSpectrumSignalParameters::GetSpreading ()
{
  return m_spreading;
}

} // namespace ns3
