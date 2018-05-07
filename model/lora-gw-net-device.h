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

#ifndef LORA_GW_NET_DEVICE_H
#define LORA_GW_NET_DEVICE_H

#include <cstring>
#include <ns3/node.h>
#include <ns3/address.h>
#include <ns3/net-device.h>
#include "lora-net-device.h"
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/traced-callback.h>
#include <ns3/nstime.h>
#include <ns3/ptr.h>
#include <ns3/mac32-address.h>
#include <ns3/generic-phy.h>
#include <list>

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
class LoRaGwNetDevice : public LoRaNetDevice
{
public:

  typedef Callback<std::tuple<uint16_t,uint8_t,uint8_t>, const Address &> SettingCallback;
  typedef Callback<void, double, const Address &> RssiCallback;
  //typedef Callback<void, Ptr<const Packet>> RxCallback;
  typedef Callback<void, const Address &> PowerCallback;
	struct DeviceInfo{
		Mac32Address address;
		uint16_t frequency;
		uint8_t datarate;
		uint8_t delay;
	};

  static TypeId GetTypeId (void);

  LoRaGwNetDevice ();
  virtual ~LoRaGwNetDevice ();

  /**
   * Notify the MAC that the PHY has finished a previously started transmission
   *
   */
  void NotifyTransmissionEnd (Ptr<const Packet>);

  /**
   * Notify the MAC that the PHY finished a reception with an error
   *
   */
  void NotifyReceptionEndError ();

  /**
   * Notify the MAC that the Powerfinished a reception successfully
   *
   * \param p the received packet
   */
  void NotifyReceptionEndOk (Ptr<Packet> p, uint32_t bandwidth, uint8_t spreading, uint32_t frequency, double rssi);

  /**
	* Get the PHY layer of this net-device
   * \return a reference to the PHY object embedded in this NetDevice.
   */
  Ptr<LoRaGwPhy> GetPhy () const;

  // inherited from NetDevice
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest,
                         uint16_t protocolNumber);
	virtual void DoInitialize ();

protected:
	/**
		* This method schedules an event whether the Acknowledgement should be send now or later. 
		* If it is send to early, a lot of packets could be interrupted.
		* 
		* \params addr address to send a message to
		* \params frequency centerfrequency of the signal
		* \params datarate datarate of the packet
		* \params powerIndex power to be used
		*/
  void CheckAckSend (const Address & addr, uint32_t frequency, uint8_t datarate, uint8_t powerIndex);
	/**
		* This method schedules an event whether the Acknowledgement should be send now or later. 
		* If it is send to early, a lot of packets could be interrupted.
		* 
		* \params packet packet to send
		* \params frequency centerfrequency of the signal
		* \params datarate datarate of the packet
		* \params powerIndex power to be used
		*/
  virtual void DoCheckAckSend (Ptr<Packet> packet, uint32_t frequency, uint8_t datarate, uint8_t powerIndex);
  //inherited
	virtual void DoDispose (void);

	/**
		* RemoveFromPending removes pending packets from the queue if they have been sent to the device
		*
		* \param addr the address of which the packet can be removed
		*/
	void RemoveFromPending (const Address& addr);
  
  /**
   * Get the datarate given a bandwidth and spreading. Assumed is that 4/5 coding is used.
	 *
	 * \params bandwidth the bandwidth of the signal
	 * \params spreading the spreading of the signal
	 * \return datarate of the signal in bits per second.
   */
	static uint8_t GetDatarate (uint32_t bandwidth, uint8_t spreading);

	/**
		* GetRxDatarate provide the datarate for the first reception slot
		* 
		* \param dr the transmission data rate
		* \param offset the offset to be used
		* \return the reception datarate
		*/
	uint8_t GetRxDatarate (uint8_t dr, uint8_t offset);

	std::list< std::tuple<Address, Ptr<Packet> > > m_pendingPackets; //!< The list of pending packets
	uint8_t m_delay; //!< the delay to retransmit a message
};


} // namespace ns3

#endif /* LORA_GW_NET_DEVICE_H */
