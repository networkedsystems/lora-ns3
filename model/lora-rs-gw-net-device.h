/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Brecht Reyndres <brecht.reynders@esat.kuleuven.be>
 */

#ifndef LORA_RS_GW_NET_DEVICE_H
#define LORA_RS_GW_NET_DEVICE_H

#include <cstring>
#include <ns3/node.h>
#include <ns3/address.h>
#include <ns3/net-device.h>
#include "lora-rs-net-device.h"
#include "lora-gw-net-device.h"
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/traced-callback.h>
#include <ns3/nstime.h>
#include <ns3/ptr.h>
#include <ns3/mac32-address.h>
#include <ns3/generic-phy.h>

namespace ns3 {

  class LoRaGwPhy;
  class SpectrumChannel;
  class Channel;
  class SpectrumErrorModel;



  /**
   * \ingroup spectrum
   *
   * This is the MAC-layer for the gateway.
   * This class can:
   *	-handle multiple simultaneous receptions
   * Most of the code is programmed in LoRaNetDevice, as lots of functionality is the same
   */
  class LoRaRsGwNetDevice : public LoRaGwNetDevice
  {
  public:

    typedef Callback<std::tuple<uint16_t,uint8_t,uint8_t>, const Address &> SettingCallback;
    typedef Callback<void, double, const Address &> RssiCallback;
    typedef Callback<void, const Address &> PowerCallback;
    struct DeviceInfo{
      Mac32Address address;
      uint8_t power;
      uint8_t datarate;
      uint8_t delay;
      //uint16_t channelMask;
    };

    static TypeId GetTypeId (void);

    LoRaRsGwNetDevice ();
    virtual ~LoRaRsGwNetDevice ();

    void DoInitialize (void);

		/**
			* This function sets the offset of the beacons.
			*
			* \param offset the requested offset
			*/
		void SetOffset (uint32_t offset);

		/**
			* This function returns the offset of the beacons of this device
			*
			* \return the offset of this netdevice
			*/
		uint32_t GetOffset (void);


    /**
     * Notify the MAC that the Powerfinished a reception successfully
     *
     * \param p the received packet
     */
    //void NotifyReceptionEndOk (Ptr<Packet> p, uint32_t bandwidth, uint8_t spreading, uint32_t frequency, double rssi);

  protected:
	/**
		* This method schedules an event whether the Acknowledgement should be send now or later. 
		* If it is send to early, a lot of packets could be interrupted.
		* 
		* \params packet packet to send
		* \params frequency centerfrequency of the signal
		* \params datarate datarate of the packet
		* \params powerIndex power to be used
		*/
  virtual void DoCheckAckSend (Ptr<const Packet> packet, uint32_t frequency, uint8_t datarate, uint8_t powerIndex);
    //inherited
    virtual void DoDispose (void);

    /**
     * start the transmission of a packet by contacting the PHY layer
     *
     */
    //void StartTransmission (Ptr<Packet> packet, uint32_t frequency, uint8_t datarate, uint8_t powerIndex);

    /**
     * Send a beacon
     */
    void SendBeacon ();

    /**
     * Get the datarate given a bandwidth and spreading. Assumed is that 4/5 coding is used.
     *
     * \params bandwidth the bandwidth of the signal
     * \params spreading the spreading of the signal
     * \return datarate of the signal in bits per second.
     */
    static uint8_t GetDatarate (uint32_t bandwidth, uint8_t spreading);

    EventId m_beacon;

    uint8_t m_rssiValues[16];
    uint8_t m_receptionsPerChannel[16];
    uint8_t m_availableChannels;

		private:
		Time m_interBeacon;
		uint32_t m_offset;

  };



} // namespace ns3

#endif /* LORA_RS_GW_NET_DEVICE_H */
