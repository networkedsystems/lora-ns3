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

#include "lora-phy-gw.h"
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

	NS_LOG_COMPONENT_DEFINE ("LoRaPhyGw");

	NS_OBJECT_ENSURE_REGISTERED (LoRaPhyGw);


	TypeId
		LoRaPhyGw::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaPhyGw")
				.SetParent<LoRaPhy> ()
				.AddConstructor<LoRaPhyGw> ()
				;
			return tid;
		}

	LoRaPhyGw::LoRaPhyGw ()
		:LoRaPhy()
	{
		NS_LOG_FUNCTION (this);
		m_collisions = 0;
	}

	LoRaPhyGw::~LoRaPhyGw ()
	{
		NS_LOG_FUNCTION (this);
	}

	void 
		LoRaPhyGw::DoDispose (void)
		{
		m_netDevice = 0;
		m_mobility = 0;
		m_txPsd = 0;
		LoRaPhy::DoDispose ();
		}

	void
		LoRaPhyGw::SetReceptionEndCallback (Callback<void,Ptr<Packet>,uint32_t,uint8_t,uint32_t,double> callback)
		{
			NS_LOG_FUNCTION (this);
			m_ReceptionEnd = callback;
		}

	bool
		LoRaPhyGw::StartTx (Ptr<Packet> packet)
		{
			NS_LOG_FUNCTION (this);
			//Add PHY header to packet
			LoRaPhyHeader lh;
			packet->AddHeader(lh);
			//Create transmitting signal parameters
			Ptr<LoRaSpectrumSignalParameters> txParams = Create<LoRaSpectrumSignalParameters> ();
			// subtract 32 as an empty packet already accounts for 32 bytes (which we are not actually transmitting)
			NS_LOG_DEBUG("The size of the actual message is " << packet->GetSize());
			//txParams->duration = Seconds(packet->GetSize()*8.0/(GetBitRate(m_spreadingfactor)));
			txParams->packet = packet;
			txParams->txPhy = this; //GetObject<SpectrumPhy> ();
			txParams->SetSpreading(m_spreadingfactor);
			txParams->SetBandwidth(m_bandwidth);
			txParams->duration = Seconds((packet->GetSize())*8.0/(GetBitRate(m_spreadingfactor)));
			txParams->psd = GetTxPowerSpectralDensity(m_channelIndex,m_power);
			txParams->txAntenna = m_antenna;
			txParams->SetChannel(m_channelIndex);
			m_transmission = true;
			// transmit it
			Simulator::Schedule(txParams->duration,&LoRaPhyGw::EndTx,this,packet->Copy());
			Simulator::ScheduleNow(&LoRaPhyGw::StartRx,this,txParams->Copy());
			m_channel->StartTx(txParams);
			return true;
		}

	void
		LoRaPhyGw::StartRx (Ptr<SpectrumSignalParameters> params)
		{
			NS_LOG_FUNCTION (this << params);
			//update BER
			UpdateBer();
			// Check if params are from lora 
			Ptr<LoRaSpectrumSignalParameters> sfParams = DynamicCast<LoRaSpectrumSignalParameters> (params);
			// add power to received power
			*m_receivingPower += *params->psd;
			//Schedule the end of the noise
			Simulator::Schedule(params->duration,&LoRaPhyGw::EndNoise,this,params->psd);
			m_ReceptionStart();
			if (sfParams != 0){
				//in this case if it is a lora packet arriving
				//Schedule end of reception
				Simulator::Schedule(sfParams->duration,&LoRaPhyGw::EndRx,this,sfParams);
				uint32_t channel = sfParams->GetChannel();
				uint32_t bandwidth = sfParams->GetBandwidth();
				//uint8_t spreading = sfParams->GetSpreading();
				//uint8_t error =0;
				//Choose highest SNR if multiple
				for (auto &i : m_params)
				{
					//Check if spreading are orthogonal, if so, then there is no problem
					if (2*i->GetBandwidth()/pow(2,i->GetSpreading()-2) == 2*sfParams->GetBandwidth()/pow(2,sfParams->GetSpreading()-2))
					{
						//Check if bandwidths collide
						if ( i->GetChannel()+i->GetBandwidth()/200> channel-bandwidth/200 && i->GetChannel()-i->GetBandwidth()/200 < channel+bandwidth/200)
						{
							m_collisions++;

							double Poweri = 0.0;
							double Powerj = 0.0;
							//check which packets gets destroyed.
							//datasheet says 6dB power difference -> power should be factor 4 higher.
							for (uint8_t p = 0; p<25; p++){
								Poweri += (*i->psd)[p];
								Powerj += (*sfParams->psd)[p];
							}
							if (Poweri > 4*Powerj)
							{
								sfParams->SetBer(10);
							}
							else
							{
								if (Powerj > 4*Poweri)
								{
									i->SetBer(10);
								}
								else
								{
									m_collisions++;
									i->SetBer(10);
									sfParams->SetBer(10);
								}
							}
						}
					}
				}
				// Check if we did not reach the maximal amount of concurrent receptions
				if (GetReceptions () > 8)
				{
					NS_LOG_DEBUG("TOO many messages in the queue. " << GetReceptions () << sfParams);
					// we are receiving too many messages at this time. This one is impossible to track.
					// Give it some bit errors such that it for sure does not arrive. Even though the SNR is good.
					sfParams->SetBer (10);
				}
				// push the sfParams in the queue. 
				m_params.push_back(sfParams);
			}
			else
			{
			}
		}

	uint32_t
		LoRaPhyGw::GetCollisions()
		{
			return m_collisions;
		}

	uint32_t 
		LoRaPhyGw::GetReceptions()
		{
			uint8_t temp = 0;
			for (auto &i : m_params)
			{
				if(i->GetBer()==0)
				{
					temp++;
				}
			}
			return temp;
		}

	void 
		LoRaPhyGw::EndRx (Ptr<LoRaSpectrumSignalParameters> params)
		{
			NS_LOG_FUNCTION(this << params);
			//update BER
			UpdateBer();
			//Remove packet from list
			std::vector<Ptr<LoRaSpectrumSignalParameters> >::iterator temp=m_params.begin();
			for (std::vector<Ptr<LoRaSpectrumSignalParameters> >::iterator it=m_params.begin(); it!=m_params.end();it++)
			{
				if (*it==params)
				{
					temp = it;
					break;
				}
			}
			m_params.erase(temp);
			NS_LOG_DEBUG("params are erased" << params << GetReceptions());
			//decide packet error or not
			if(params->GetBer()<5)
			{
				//no packet error
				LoRaPhyHeader lh;
				Ptr<Packet> packet = params->packet;
				packet->RemoveHeader(lh);
				m_ReceptionEnd( packet, params->GetBandwidth(), params->GetSpreading(), params->GetChannel(),(*params->psd)[(params->GetChannel()-868e4)/250+1]*params->GetBandwidth() );
			}
			else
			{
				//packet error
				m_ReceptionError();
			}
		}


	void 
		LoRaPhyGw::UpdateBer ()
		{
			NS_LOG_FUNCTION(this);
			double timeNow = Simulator::Now().GetSeconds(); 
			for (auto &i : m_params)
			{
				//calculate SNR
				if (i->GetBer() < 10)
				{
					Ptr<SpectrumValue> noise = m_receivingPower->Copy();
					*noise -= *i->psd; 
					double signalPower = 0.0;
					double noisePower = 0.0;
					uint32_t bandwidth = i->GetBandwidth();
					uint32_t freq = i->GetChannel();
					for ( int k = (freq-868e4-bandwidth/200)/250+1; k<(freq-868e4+bandwidth/200)/250+1; k++){
						signalPower += (*i->psd)[k];
						noisePower += ((*noise)[k]+m_k*m_temperature);
					}

					double snr = signalPower/noisePower;
					//getBER
					long double berEs = m_errorModel->GetBER (snr, i->GetSpreading(), m_bandwidth);
					double m_bitErrors = i->GetBer();
					SetSpreadingFactor(i->GetSpreading());
					SetBandwidth(i->GetBandwidth());
					uint16_t bits = (timeNow - m_lastCheck)*GetBitRate(m_spreadingfactor);
					for ( uint16_t it = 0; it<bits; it++)
					{
						if(m_random->GetValue()<berEs)
						{
							m_bitErrors+=1;
						}
					}
					if (m_bitErrors>0)
					{
						NS_LOG_DEBUG(snr << " " << signalPower << " " << noisePower << " " <<  bits << " " << GetBitRate(m_spreadingfactor) << " " << m_bitErrors << " " << berEs);
					}
					//calculate numbers of biterrors	
					i->SetBer(m_bitErrors);
				}
			}
			m_lastCheck = timeNow;
		}

} // namespace
