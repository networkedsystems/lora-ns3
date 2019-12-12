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

#ifndef NOISE_ISM_H
#define NOISE_ISM_H


#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-phy.h>
#include <ns3/event-id.h>
#include <ns3/random-variable-stream.h>
#include <ns3/callback.h>
#include <ns3/traced-value.h>
namespace ns3 {

class SpectrumChannel;
class MobilityModel;
class AntennaModel;
class SpectrumValue;
class SpectrumModel;
class NetDevice;
struct SpectrumSignalParameters;

/**
 * \ingroup lora
 *
 * 868 MHz ISM noise implementation 
 *
 */

class NoiseIsm : public SpectrumPhy
{

public:

	NoiseIsm ();
  ~NoiseIsm ();

  static TypeId GetTypeId (void);
  

  /**
   * set the associated NetDevice instance
   *
   * \param d the NetDevice instance
   */
  void SetDevice (Ptr<NetDevice> d);
  Ptr<NetDevice> GetDevice () const;


  /**
   * Set the mobility model associated with this device.
   *
   * \param m the mobility model
   */
  void SetMobility (Ptr<MobilityModel> m);

  /**
   * get the associated MobilityModel instance
   *
   * \return a Ptr to the associated MobilityModel instance
   */
  Ptr<MobilityModel> GetMobility ();

  /**
   * Set the channel attached to this device.
   *
   * \param c the channel
   */
  void SetChannel (Ptr<SpectrumChannel> c);

  /**
   *
   * \return returns the SpectrumModel that this SpectrumPhy expects to be used
   * for all SpectrumValues that are passed to StartRx. If 0 is
   * returned, it means that any model will be accepted.
   */
  Ptr<const SpectrumModel> GetRxSpectrumModel () const;
  void SetRxSpectrumModel (Ptr<const SpectrumModel> model);

  /**
   * Set the power spectrum by setting channel and power
   *
   * \param channeloffset number of channel used
   * \param power total radiated power
   */
   void InitPowerSpectralDensity ();

  /**
   * get the AntennaModel used by the NetDevice for reception
   *
   * \return a Ptr to the AntennaModel used by the NetDevice for reception
   */
  Ptr<AntennaModel> GetRxAntenna ();
  void SetRxAntenna (Ptr<AntennaModel> a);

  /**
   * Notify the SpectrumPhy instance of an incoming signal
   *
   * \param params the parameters of the signals being received
   */
  void StartRx (Ptr<SpectrumSignalParameters> params);

	void StartNoise (void);

private:

  Ptr<NetDevice> m_netDevice; //!<upper layer
  Ptr<MobilityModel> m_mobility; //!<position
  Ptr<SpectrumChannel> m_channel; //!<channel to transmit on
  Ptr<SpectrumValue> m_rxPsd; //!<Current psd
  Ptr<AntennaModel> m_antenna; //!<antenna to be used

  Ptr<RandomVariableStream> m_bandwidth; //!< The bandwidth of a random signal 
  Ptr<RandomVariableStream> m_centerFrequency; //!< The center frequency of a random signal 
  Ptr<RandomVariableStream> m_length; //!< The length of a packet
  Ptr<RandomVariableStream> m_time; //!< The start of a new message.

  double m_power;  
 
  EventId m_event; //!< When the receiving packet is being received
  /**
  * Create spectrumValues for LoRa signal
  *
  * \param channeloffset tells system where signal is situated in spectrum
  * \param power tells system how strong the signal is (in W)
  *
  * \return spectrum value for LoRa signal
  */
  Ptr<SpectrumValue> CreateTxPowerSpectralDensity (void);
  
  /**
   * Send noise on the channel.
   * This function is meant to be scheduled.
   *
   */
  void SendNoise ();
};
} // namespace ns3
#endif /* NOISE_ISM_H */
