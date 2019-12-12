/* -*- Mode:Ci++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "noise-ism.h"
#include <ns3/spectrum-phy.h>
#include <ns3/log.h>
#include <ns3/spectrum-model.h>
#include <ns3/spectrum-value.h>
#include <ns3/antenna-model.h>
#include <ns3/simulator.h>
#include <ns3/event-id.h>
#include <ns3/random-variable-stream.h>
#include "ns3/pointer.h"
#include "ns3/string.h"
#include <ns3/net-device.h>
#include "ns3/mobility-model.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NoiseIsm");

NS_OBJECT_ENSURE_REGISTERED (NoiseIsm);


TypeId
NoiseIsm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NoiseIsm")
    .SetParent<SpectrumPhy> ()
    .AddConstructor<NoiseIsm> ()
    .AddAttribute ("Bandwidth",
                   "The random variable for the bandwidth of the signal.",
                   PointerValue (0),
                   MakePointerAccessor (&NoiseIsm::m_bandwidth),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("CenterFrequency",
                   "The random variable for the center frequency of the signal.",
                   PointerValue (0),
                   MakePointerAccessor (&NoiseIsm::m_centerFrequency),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("MessageLength",
                   "A random variable for sampling the length.",
                   PointerValue (0),
                   MakePointerAccessor (&NoiseIsm::m_length),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("StartTime",
                   "A random variable for the start of the messages (This is G in ALOHA formulas)",
                   PointerValue (0),
                   MakePointerAccessor (&NoiseIsm::m_time),
                   MakePointerChecker<RandomVariableStream> ())
  ;
  return tid;
}

NoiseIsm::NoiseIsm ()
{
  NS_LOG_FUNCTION (this);
  m_antenna = 0;
  m_mobility = 0;
  m_channel = 0;
  m_netDevice = 0;
  m_power = 0.025;
}

NoiseIsm::~NoiseIsm ()
{
  NS_LOG_FUNCTION (this);
  m_netDevice = 0;
  m_mobility = 0;
  m_channel = 0;
  m_antenna = 0;
  m_bandwidth = 0;
	m_time = 0;
	m_bandwidth=0;
	m_centerFrequency = 0;
	m_length = 0;
}

void
NoiseIsm::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this);
  m_netDevice = d;
}

Ptr<NetDevice>
NoiseIsm::GetDevice () const
{
  NS_LOG_FUNCTION (this);
  return m_netDevice;
}

void
NoiseIsm::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this);
  m_mobility = m;
}

Ptr<MobilityModel>
NoiseIsm::GetMobility ()
{
	NS_LOG_FUNCTION(this);
  return m_mobility;
}

void
NoiseIsm::SetChannel (Ptr<SpectrumChannel> c)
{
  NS_LOG_FUNCTION (this);
  m_channel = c;
}

void
NoiseIsm::SetRxSpectrumModel (Ptr<const SpectrumModel> model)
{
	NS_LOG_FUNCTION(this);
	m_rxPsd = 0;
	m_rxPsd = Create <SpectrumValue> (model);
}

Ptr<SpectrumValue>
NoiseIsm::CreateTxPowerSpectralDensity (void)
{
  NS_LOG_FUNCTION(this);
  // Sample center frequency, bandwidth
  double bandwidth = 0;
	while (bandwidth <= 0)
		bandwidth = m_bandwidth->GetInteger();
  double txPowerDensity = 0.025/bandwidth;
  double channelwidth = bandwidth/4;
  double channeloffset =0;
	while (channeloffset <= 0)
		channeloffset = m_centerFrequency->GetValue();
  //and fill it
  Bands bands;
  for (uint32_t i= 0; i < 4 ;i++){
		BandInfo bi;
		bi.fl = channeloffset+i*channelwidth-bandwidth/2;
 		bi.fh = channeloffset+(i+1)*channelwidth-bandwidth/2;
		bi.fc = (bi.fl+bi.fh)/2;
		bands.push_back (bi);
  }
  Ptr<SpectrumModel> sm = Create<SpectrumModel> (bands);
  Ptr<SpectrumValue> test = Create<SpectrumValue> (sm);
  for (uint32_t i= 0; i < 4;i++){
		(*test)[i]=txPowerDensity;
	}
  return test->Copy ();
}

Ptr<const SpectrumModel>
NoiseIsm::GetRxSpectrumModel () const
{
  NS_LOG_FUNCTION (this);
  return m_rxPsd->GetSpectrumModel ();
}

void
NoiseIsm::SetRxAntenna (Ptr<AntennaModel> a)
{
  NS_LOG_FUNCTION (this);
  m_antenna = a;
}

Ptr<AntennaModel>
NoiseIsm::GetRxAntenna ()
{
  NS_LOG_FUNCTION (this);
  return m_antenna;
}

	void
NoiseIsm::StartRx (Ptr<SpectrumSignalParameters> params)
{
	NS_LOG_FUNCTION (this);
	// DO nothing
}

void 
NoiseIsm::SendNoise ()
{
	NS_LOG_FUNCTION(this);
   Ptr<SpectrumSignalParameters> txParams = Create<SpectrumSignalParameters> ();
	 double duration = 0;
	 while (duration <= 0)
		 duration = m_length->GetValue();
   txParams->duration = Seconds(duration);
   txParams->txPhy = this;
   txParams->psd = CreateTxPowerSpectralDensity();
   txParams->txAntenna = m_antenna;
   m_channel->StartTx (txParams);
   Simulator::Schedule(Seconds(std::max(0.0,m_time->GetValue())),&NoiseIsm::SendNoise,this);
}

void 
NoiseIsm::StartNoise (void)
{
	NS_LOG_FUNCTION(this);
   Simulator::Schedule(Seconds(std::max(0.0,m_time->GetValue())),&NoiseIsm::SendNoise,this);
}

} // namespace
