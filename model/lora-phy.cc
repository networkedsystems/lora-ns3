/* -*- Mode:Ci++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "lora-phy.h"
#include "lora-spectrum-signal-parameters.h"
#include "lora-phy-header.h"
#include <ns3/object.h>
#include <ns3/spectrum-phy.h>
#include <ns3/net-device.h>
#include <ns3/mobility-model.h>
#include <ns3/spectrum-value.h>
#include <ns3/spectrum-channel.h>
#include <ns3/log.h>
#include <ns3/spectrum-model.h>
#include <ns3/antenna-model.h>
#include <ns3/simulator.h>
#include <ns3/event-id.h>
#include <ns3/random-variable-stream.h>
#include <ns3/double.h>
#include <cmath>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoRaPhy");

NS_OBJECT_ENSURE_REGISTERED (LoRaPhy);


TypeId
LoRaPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaPhy")
    .SetParent<SpectrumPhy> ()
    .AddConstructor<LoRaPhy> ()
				.AddTraceSource ("StateValue",
						"The state of the transceiver",
						MakeTraceSourceAccessor (&LoRaPhy::m_state),
						"ns3::TracedValueCallback::State")
  ;
  return tid;
}

LoRaPhy::LoRaPhy ()
{
  NS_LOG_FUNCTION (this);
  m_state = IDLE;
  m_k = 1.38e-23;
  m_temperature = 298;
  m_bandwidth = 125000;
  m_antenna = 0;
  m_spreadingfactor = 12;
  m_params=0;
  m_mobility = 0;
  m_channelIndex = 868e4;
  m_channel = 0;
  m_netDevice = 0;
  m_random=CreateObject<UniformRandomVariable> ();
  m_random->SetAttribute ("Min",DoubleValue(0.0));
  m_random->SetAttribute ("Max",DoubleValue(1.0));
  m_power = 0.025;
  m_bitErrors = 0;
	m_transmission = false;
  m_errorModel =Create<LoRaErrorModel> (); 
  InitPowerSpectralDensity ();
  m_receivingPower = Create<SpectrumValue> (m_rxPsd->GetSpectrumModel ());
  for (uint8_t i = 0; i<8; i++)
  {
    for (uint8_t j=0; j<30; j++)
    {
      m_channelUsage[i][j] = 0;
    }
  }
}

LoRaPhy::~LoRaPhy ()
{
  NS_LOG_FUNCTION (this);
  m_netDevice = 0;
  m_mobility = 0;
  m_channel = 0;
  m_antenna = 0;
  m_rxPsd = 0;
	m_errorModel = 0;
}

void
LoRaPhy::ChangeState (State state)
{
  NS_LOG_FUNCTION (this << state);
	if (state == IDLE)
	{
		Simulator::Remove(m_event);
		m_params = 0;
	}
  m_state = state;
	// stop EndRx if phy is stopped
}


void
LoRaPhy::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this);
  m_netDevice = d;
}

Ptr<NetDevice>
LoRaPhy::GetDevice () const
{
  NS_LOG_FUNCTION (this);
  return m_netDevice;
}

void
LoRaPhy::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this);
  m_mobility = m;
}

Ptr<MobilityModel>
LoRaPhy::GetMobility ()
{
  return m_mobility;
}

void
LoRaPhy::SetChannel (Ptr<SpectrumChannel> c)
{
  NS_LOG_FUNCTION (this);
  c->AddRx(this);
  m_channel = c;
}

void
LoRaPhy::InitPowerSpectralDensity ()
{
  NS_LOG_FUNCTION (this);
  // create offset
  Bands bands;
  for (int i= 0; i < 70;i++){
	BandInfo bi;
	bi.fl = 868e6+i*25000;
 	bi.fh = 868e6+(i+1)*25000;
	bi.fc = (bi.fl+bi.fh)/2;
	bands.push_back (bi);
  }
  Ptr<SpectrumModel> sm = Create<SpectrumModel> (bands);
  m_rxPsd = Create <SpectrumValue> (sm);
}

void
LoRaPhy::SetRxSpectrumModel (Ptr<const SpectrumModel> model)
{
	m_rxPsd = 0;
	m_rxPsd = Create <SpectrumValue> (model);
  m_receivingPower = Create<SpectrumValue> (m_rxPsd->GetSpectrumModel ());
}

Ptr<SpectrumValue>
LoRaPhy::GetFullTxPowerSpectralDensity (uint32_t channeloffset, double power)
{
  NS_LOG_FUNCTION(this);
  // 3 banden, waarvan een heel groot deel niet gebruikt worden. 
  double txPowerDensity = power/m_bandwidth;
  //reset m_rxPsd
  for (uint32_t i = 0; i<70; i++)
  {
    (*m_rxPsd)[i]=0;
  }
  //and fill it
  for (uint8_t j = (channeloffset-868e4)/250-m_bandwidth/2/25000; j<(channeloffset-868e4)/250+m_bandwidth/2/25000+1; j++){
  	(*m_rxPsd)[j] = txPowerDensity;
  }
  return m_rxPsd->Copy ();
}

Ptr<SpectrumValue>
LoRaPhy::GetTxPowerSpectralDensity (uint32_t channeloffset, double power)
{
  NS_LOG_FUNCTION(this);
  // 3 banden, waarvan een heel groot deel niet gebruikt worden. 
  double txPowerDensity = power/m_bandwidth;
  //and fill it
  Bands bands;
  for (int i= 0; i < m_bandwidth/25e3;i++){
		BandInfo bi;
		bi.fl = channeloffset*100+i*25000-m_bandwidth/2;
 		bi.fh = channeloffset*100+(i+1)*25000-m_bandwidth/2;
		bi.fc = (bi.fl+bi.fh)/2;
		bands.push_back (bi);
  }
  Ptr<SpectrumModel> sm = Create<SpectrumModel> (bands);
	Ptr<SpectrumValue> test = Create<SpectrumValue> (sm);
  for (int i= 0; i < m_bandwidth/25e3;i++){
		(*test)[i]=txPowerDensity;
	}
  return test->Copy ();
}

Ptr<const SpectrumModel>
LoRaPhy::GetRxSpectrumModel () const
{
  NS_LOG_FUNCTION (this);
  return m_rxPsd->GetSpectrumModel ();
}

void
LoRaPhy::SetRxAntenna (Ptr<AntennaModel> a)
{
  NS_LOG_FUNCTION (this);
  m_antenna = a;
}

Ptr<AntennaModel>
LoRaPhy::GetRxAntenna ()
{
  NS_LOG_FUNCTION (this);
  return m_antenna;
}

void
LoRaPhy::SetChannelIndex (uint32_t channel)
{
	NS_LOG_FUNCTION (this);
    m_channelIndex = channel;
}

uint32_t
LoRaPhy::GetChannelIndex (void)
{
	NS_LOG_FUNCTION (this);
	return m_channelIndex;
}

int
LoRaPhy::GetBitRate (uint8_t sf) {
	NS_LOG_FUNCTION (this);
	return round(m_bandwidth*sf/pow(2,sf));
}

Time
LoRaPhy::GetTimeOfPacket (uint16_t length,uint8_t sf)
{
	NS_LOG_FUNCTION (this);
	LoRaPhyHeader lh;
	return Seconds((double)(length+lh.GetSerializedSize())/(double)GetBitRate(sf)*8);
}

	bool
LoRaPhy::IsTransmitting ()
{
	NS_LOG_FUNCTION (this);
	return m_transmission;
}

	double 
LoRaPhy::GetRssiLastPacket (void)
{
	NS_LOG_FUNCTION (this);
	return m_lastRssi;
}
	
	double 
LoRaPhy::GetSnrLastPacket (void)
{
	NS_LOG_FUNCTION (this);
	return m_lastSnr;
}
void 
LoRaPhy::SetSpreadingFactor (uint8_t sf){
	NS_LOG_FUNCTION (this);
	m_spreadingfactor = sf;
}

	void
LoRaPhy::SetBandwidth (uint32_t bandwidth)
{
	NS_LOG_FUNCTION (this);
	m_bandwidth = bandwidth;
}

	void
LoRaPhy::SetPower (double power)
{
	NS_LOG_FUNCTION (this);
	m_power = power;
}

	void
LoRaPhy::SetTransmissionEndCallback (Callback<void, Ptr<const Packet> > callback)
{
	NS_LOG_FUNCTION (this);
	m_transmissionEnd = callback;
}

	void
LoRaPhy::SetReceptionStartCallback (Callback<void> callback)
{
	NS_LOG_FUNCTION (this);
	m_ReceptionStart = callback;
}

	void 
LoRaPhy::SetReceptionErrorCallback (Callback<void> callback)
{
	NS_LOG_FUNCTION (this);
	m_ReceptionError = callback;
}

	void
LoRaPhy::SetReceptionEndCallback (Callback<void,Ptr<Packet>,double> callback)
{
	NS_LOG_FUNCTION (this);
	m_ReceptionEnd = callback;
}

	void
LoRaPhy::SetReceptionMacCallback (Callback<void,LoRaMacHeader> callback)
{
	NS_LOG_FUNCTION(this);
	m_ReceptionMacEnd = callback;
}

	bool
LoRaPhy::StartTx (Ptr<Packet> packet)
{
	NS_LOG_FUNCTION (this);
	Ptr<Packet> copy = packet->Copy();
	if (m_state == TX)
	{
		//Add PHY header to packet
		LoRaPhyHeader lh;
		copy->AddHeader(lh);
		//Create transmitting signal parameters
		Ptr<LoRaSpectrumSignalParameters> txParams = Create<LoRaSpectrumSignalParameters> ();
		// subtract 32 as an empty packet already accounts for 32 bytes (which we are not actually transmitting)
		txParams->duration = Seconds((copy->GetSize())*8.0/(GetBitRate(m_spreadingfactor)));
		Simulator::Schedule(txParams->duration,&LoRaPhy::EndTx,this,copy->Copy());
		txParams->packet = copy;
		txParams->txPhy = GetObject<SpectrumPhy> ();
		txParams->SetSpreading(m_spreadingfactor);
		txParams->SetBandwidth(m_bandwidth);
		txParams->psd = GetTxPowerSpectralDensity(m_channelIndex,m_power);
		txParams->txAntenna = m_antenna;
		txParams->SetChannel(m_channelIndex);
		m_transmission = true;
		m_channel->StartTx (txParams);
		return true;
	}
	return false;
}

	void 
LoRaPhy::EndTx (Ptr<Packet> packet)
{
	NS_LOG_FUNCTION (this);
	m_state = IDLE;
	m_transmission = false;
	LoRaPhyHeader lh;
	packet->RemoveHeader(lh);
	m_transmissionEnd (packet);
}

	void
LoRaPhy::StartRx (Ptr<SpectrumSignalParameters> params)
{
	NS_LOG_FUNCTION (this);
	//update BER
	UpdateBer();
	//Check if params are from LoRa
	Ptr<LoRaSpectrumSignalParameters> sfParams = DynamicCast<LoRaSpectrumSignalParameters> (params);
	// add power to received power
	*m_receivingPower += *params->psd;
	//Schedule the end of the noise
	Simulator::Schedule(params->duration,&LoRaPhy::EndNoise,this,params->psd);
	if (sfParams != 0){
		//Get parameters of the current received signal parameters
		uint32_t channel = sfParams->GetChannel();
		uint32_t bandwidth = sfParams->GetBandwidth();
		uint8_t spreading = sfParams->GetSpreading();
		NS_LOG_DEBUG (spreading << " " << bandwidth << " " << (uint32_t) channel );
		NS_ASSERT(channel > 8680000);
		NS_ASSERT(bandwidth > 100000);
		uint8_t error = 0;
		//Add signal to channel usage matrix
		for (uint8_t i = (channel-8680000-bandwidth/200)/250; i <= (channel-8680000+bandwidth/200)/250; i++){
			if ((*sfParams->psd)[i]!=0){
				error = error + (m_channelUsage[((12-spreading)+(bandwidth/125000-1))][i]>(*sfParams->psd)[i]);
				m_channelUsage[((12-spreading)+(bandwidth/125000-1))][i]=(m_channelUsage[((12-spreading)+(bandwidth/125000-1))][i]+(*sfParams->psd)[i]);
			}
		}
		// Remove interfering signals from the list
		Simulator::Schedule(sfParams->duration,&LoRaPhy::EndSignal,this,sfParams->psd,channel,spreading,bandwidth);
		// if there is already something transmitting it should be checked if the new values interfere with the current signal parameters
		if (m_params!=0)
		{
			double powerInterferer = 0.0;
			double powerReceiver = 0.0;
			for (uint8_t i = (channel-8680000-bandwidth/200)/250; i<= (channel-8680000+bandwidth/200)/250; i++)
			{
				powerInterferer+= (*sfParams->psd)[i];
				powerReceiver+= (*m_params->psd)[i];
			}
			//if the parameters fall together
			if (m_params->GetChannel() - channel < (m_params->GetBandwidth()+bandwidth)/200 && (12-spreading + bandwidth/125000-1) == (12-m_params->GetSpreading()+m_params->GetBandwidth()/125000-1) && powerInterferer * 4 > powerReceiver)
			{
				//packet is lost
				Simulator::Remove(m_event);
				m_ReceptionError();
				m_params = 0;
				error ++;
				m_state = IDLE;
			}
		}
		//if the settings of the transceiver are ok, then continue
		NS_LOG_DEBUG((uint32_t)error << RX << m_state << m_channelIndex << channel << m_bandwidth << sfParams->GetBandwidth() << (uint32_t)m_spreadingfactor  << sfParams->GetSpreading());
		if (error ==0 && m_state == RX && m_channelIndex == channel && m_bandwidth == sfParams->GetBandwidth() && m_spreadingfactor == sfParams->GetSpreading())
		{
			if (m_params==0)
			{
				m_params=sfParams;
				m_bitErrors=0;
				//generate ending event
				m_event =Simulator::Schedule(sfParams->duration,&LoRaPhy::EndRx,this,sfParams);
				if(sfParams->duration.GetSeconds() > 17.0*8.0/GetBitRate(m_spreadingfactor))
				{
					Simulator::Schedule(Seconds(17.0*8.0/GetBitRate(m_spreadingfactor)),&LoRaPhy::SendMac,this);
				}
				m_ReceptionStart();
			}
		}
	}
}

	void
LoRaPhy::SendMac ()
{
	NS_LOG_FUNCTION (this);
	// check if receiving and if there is a packet
	if(m_state==RX && m_params!=0 && m_bitErrors == 0){
		// copy the packet to make sure nothing changes
		Ptr<Packet> packet = m_params->packet->Copy();
		// remove header
		LoRaPhyHeader lp;
		packet->RemoveHeader(lp);
		LoRaMacHeader lm;
		packet->RemoveHeader(lm);
		// send mac header to mac layer
		//m_ReceptionMacEnd(lm);
	}
}

	void
LoRaPhy::EndSignal (Ptr<SpectrumValue> sv, uint32_t frequency, uint8_t spreading, uint32_t bandwidth)
{
	NS_LOG_FUNCTION (this);
	// Remove signal from detailed receiving list
	for (uint8_t i = (frequency-8680000-bandwidth/200)/250; i <= (frequency-8680000+bandwidth/200)/250; i++){
		m_channelUsage[(((12-spreading)+(bandwidth/125000-1)))][i]= m_channelUsage[(((12-spreading)+(bandwidth/125000-1)))][i]-(*sv)[i];
	} 
}

	void
LoRaPhy::EndNoise (Ptr<SpectrumValue> sv)
{
	NS_LOG_FUNCTION (this);
	//update BER because noise is changing
	this->UpdateBer();
	// remove noise source
	*m_receivingPower -= *sv;
}

	void 
LoRaPhy::EndRx (Ptr<LoRaSpectrumSignalParameters> params)
{
	NS_LOG_FUNCTION(this);
	//update BER
	UpdateBer();
	//Reception has ended, so clear receiving parameters
	m_params=0;
	//decide packet error or not
	Ptr<Packet> packet = params->packet;
	// remove lora header
	LoRaPhyHeader lh;
	packet->RemoveHeader(lh);
	//TODO: do something with the header ( for now this is proprietary )
	//Check on bit errrors
	if(params->GetBer()<1)
	{
		//no packet error
		m_lastRssi = 10*std::log10(1250*(*params->psd)[(params->GetChannel()-868e4)/250+1]);
		m_ReceptionEnd(params->packet,params->GetBer());
	}
	else
	{
		//packet error
		m_ReceptionError();
	}
	// turn of receiver
	m_state = IDLE;
}

	void 
LoRaPhy::UpdateBer ()
{
	NS_LOG_FUNCTION (this);
	// save current Time
	double timeNow = Simulator::Now().GetSeconds(); 
	// if nothing in receing parameters, there is nothing to do
	if (m_params!=0)
	{
		//calculate SNR
		Ptr<SpectrumValue> noise = m_receivingPower->Copy();
		*noise -= *m_params->psd; 
		double signalPower = 0.0;
		uint32_t bandwidth = m_params->GetBandwidth();
		uint32_t freq = m_params->GetChannel();
		double noisePower = 0.0;
		for ( int k = (freq-868e4-bandwidth/200)/250; k<(freq-868e4+bandwidth/200)/250; k++){
			signalPower += (*m_params->psd)[k];
			noisePower += ((*noise)[k]+m_k*m_temperature);
		}

		double snr = signalPower/noisePower;
		// guard this, because this function is called twice at the end of a packet
		if (timeNow - m_lastCheck > 0.001)
		{		
			m_lastSnr = 10*std::log10(snr);
		}
		NS_LOG_DEBUG("The SNR: " << snr << " " << signalPower << " " << noisePower);
		//getBER
		long double berEs = m_errorModel->GetBER (snr, m_params->GetSpreading(), m_bandwidth);
		//calculate numbers of biterrors
		uint16_t bits = (timeNow-m_lastCheck) * GetBitRate(m_spreadingfactor);
		m_bitErrors = m_params->GetBer();
		for (uint16_t it = 0; it<bits; it++)
		{
			if(m_random->GetValue()<berEs)
			{
				m_bitErrors+=1;
			}
		}
		m_params->SetBer(m_bitErrors);
	}
	//update time 
	m_lastCheck = timeNow;
}

} // namespace
