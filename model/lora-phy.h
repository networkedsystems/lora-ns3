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

#ifndef LORA_PHY_H
#define LORA_PHY_H


#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-phy.h>
#include "lora-error-model.h"
#include "lora-spectrum-signal-parameters.h"
#include "lora-mac-header.h"
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
 * LoRa Physical layer implentation
 *
 */

class LoRaPhy : public SpectrumPhy
{

public:

	/**
 	* State of the transceiver
 	*/

	enum State
	{
  	TX,RX,IDLE
	};


	LoRaPhy ();
  ~LoRaPhy ();

  static TypeId GetTypeId (void);

  /**
   * set the associated NetDevice instance
   *
   * \param d the NetDevice instance
   */
  void SetDevice (Ptr<NetDevice> d);

  /**
   * get the associated NetDevice instance
   *
   * \return a Ptr to the associated NetDevice instance
   */
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

  /**
   * Verify if packet is correctly received. 
   *
   * \param params the parameters of the signals being received
   */
  void EndRx (Ptr<LoRaSpectrumSignalParameters> params);

  /**
   * Remove noise from the received signals
   *
   * \param sv spectrum value of the noise
   */
  void EndNoise (Ptr<SpectrumValue> sv);

  /**
   * Remove a signal from the receiving list. This is for LoRa signals
   *
   * \param frequency the carrier frequency of the signal 
   * \param spreading the spreading of the lora signal
   * \param bandwidth the bandwidth of the signal 
   */
  void EndSignal (Ptr<SpectrumValue> sv, uint32_t frequency, uint8_t spreading, uint32_t bandwidth);

  /**
   * Set callback function when packet has been transmitted
   */
  void SetTransmissionEndCallback(Callback<void,Ptr<const Packet> > callback);
 
  /**
   * Set Callback when Reception starts
   */
  void SetReceptionStartCallback(Callback<void> callback);
  
  /**
   * Set Callback when reception ends
   */
  void SetReceptionEndCallback(Callback<void,Ptr<Packet>,double> callback);
  
  /**
   * Set callback when reception has encountered an error
   */
  void SetReceptionErrorCallback(Callback<void> callback);


  void SetReceptionMacCallback (Callback<void,LoRaMacHeader> callback);
  /**
   * Start transmitting
   *
   * \params packet packet to be send
   */
  bool StartTx (Ptr<Packet> packet);

  /**
   * End transmitting. Puts transceiver back in idle mode
   */
  void EndTx (Ptr<Packet> packet);
 
  /**
   * Set Channel to send/receive on
   *
   * \params channel centrum frequency, coded in the same way as the LoRaWan spec (3 bytes ) (*100Hz)
   */
  void SetChannelIndex (uint32_t channel);
  uint32_t GetChannelIndex (void);

  /**
   * Set spreading factor of the transceiver
   *
   * \params sf spreading factor
   */
  void SetSpreadingFactor (uint8_t sf);

  /**
   * Set bandwidth of the transceiver
   *
   * \params bandwidth bandwidth of the signal
   */
  void SetBandwidth (uint32_t bandwidth);

  /**
   * Set power of the transmitter
   *
   * \params power power of the transmitter
   */
  void SetPower (double power);

  /**
   * get the bitrate of the current settings
   *
   * \return bitrate based on spreadingfactor and bandwidth
   */
  int GetBitRate (uint8_t sf);

	/**
		* get the transmission time of a packet
		*
		* \return the time of a packet in Seconds.
		*/
	Time GetTimeOfPacket(uint16_t length,uint8_t sf);

  /**
   * Set spreading factor of the transceiver
   *
   * \params sf spreading factor
   */
  void ChangeState (State state);

  /**
   * Get Transmit power spectral density for a given channel and power and the stored bandwidth and spreading
   *
   * \params channeloffset carrierfrequency to be used
   * \params power to be used to transmit
   * \return PSD of the transceiver with the current settings
   */
  Ptr<SpectrumValue> GetTxPowerSpectralDensity (uint32_t channeloffset, double power);
  
	/**
   * Get Full Transmit power spectral density for a given channel and power and the stored bandwidth and spreading
   *
   * \params channeloffset carrierfrequency to be used
   * \params power to be used to transmit
   * \return PSD of the transceiver with the current settings over the total bandwidth of the receiver.
   */
  Ptr<SpectrumValue> GetFullTxPowerSpectralDensity (uint32_t channeloffset, double power);

	/**
		* Return the received signal strength of the last received packet.
		*
		* \return the rssi of the last packet.
		*/
	double GetRssiLastPacket (void);
	
	/**
		* Return the snr of the last received packet.
		*
		* \return the snr of the last packet.
		*/
	double GetSnrLastPacket (void);

	/**
		* Check if physical layer is transmitting 
		* 
		* \return true if transmitting
		*/
	bool IsTransmitting ();

protected:

 Ptr<NetDevice> m_netDevice; //!<upper layer
 Ptr<MobilityModel> m_mobility; //!<position
 Ptr<SpectrumChannel> m_channel; //!<channel to transmit on
 Ptr<SpectrumValue> m_rxPsd; //!<Current psd
 Ptr<AntennaModel> m_antenna; //!<antenna to be used


 bool m_transmission; //!< The transceiver is actually transmitting (not in TX state)
 double m_k; //!< boltzman
 double m_temperature; //!< noise temperature
 uint32_t m_bandwidth; //!< bandwith
 uint8_t m_spreadingfactor; //!< spreading factor number
 double m_power; //!< power of transmission
 uint32_t m_channelIndex; //!< channel to transmit on
 Ptr<LoRaSpectrumSignalParameters> m_params; //!< signal parameters of the packet being received
 //Ptr<LoRaSpectrumSignalParameters> m_param; //!< 
 double m_bitErrors; //!< biterrors collected 
 double m_lastCheck; //!< last time check
 Ptr<SpectrumValue> m_receivingPower; //!< all the power at the receiving antenna
 double m_channelUsage [8][30]; //!< table with all the information of the current transmissions
 Ptr<LoRaErrorModel> m_errorModel; //!< error model for this device
 Ptr<UniformRandomVariable> m_random; //!< determines whether received package is lost are not
 //callbackfunctions
 Callback<void, Ptr<const Packet> > m_transmissionEnd; 
 Callback<void> m_ReceptionStart;
 Callback<void> m_ReceptionError;
 Callback<void, Ptr<Packet>,double> m_ReceptionEnd;

private:
 
 EventId m_event; //!< When the receiving packet is being received
 TracedValue<State> m_state; //!< state of the transceiver
 double m_lastRssi; //!< rssi of last packet
 double m_lastSnr; //!< snr of last packet
  /**
  * Create spectrumValues for LoRa signal
  *
  * \param channeloffset tells system where signal is situated in spectrum
  * \param power tells system how strong the signal is (in W)
  *
  * \return spectrum value for LoRa signal
  */
  void CreateTxPowerSpectralDensity (uint32_t channeloffset, double power);

  /**
   * Send the MAC header to the MAC-layer when this is received, this is needed to open the second receive slot.
   */
  void SendMac();
  // callback function to send the macheader.
 Callback<void, LoRaMacHeader > m_ReceptionMacEnd; 

  /**
   * Update the BER for all receiving transmissions based on latest information 
   */
  virtual void UpdateBer (void);
};
} // namespace ns3
#endif /* LORA_PHY_H */
