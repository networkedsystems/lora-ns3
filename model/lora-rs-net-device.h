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
 * Author: Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#ifndef LORA_RS_NET_DEVICE_H
#define LORA_RS_NET_DEVICE_H

#include <cstring>
#include <ns3/node.h>
#include <ns3/address.h>
#include <ns3/net-device.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/traced-callback.h>
#include <ns3/nstime.h>
#include <ns3/ptr.h>
#include <ns3/mac32-address.h>
#include <ns3/generic-phy.h>
#include <ns3/random-variable-stream.h>
#include <ns3/event-id.h>

namespace ns3 {

class LoRaPhy;
class SpectrumChannel;
class Channel;
class SpectrumErrorModel;
class Queue;
class LoRaMacHeader;
class LoRaMacCommand;



/**
 * \ingroup lora
 *
 * This devices implements the following features:
 *  - MAC addressing
 *  - Aloha MAC with beaconed scheduling
 *    + packets transmitted as soon as possible
 *    + a new packet is queued if previous one is still being transmitted
 *
 */
class LoRaRsNetDevice : public LoRaNetDevice
{
public:

  static TypeId GetTypeId (void);

  LoRaRsNetDevice ();
  virtual ~LoRaRsNetDevice ();

	virtual void DoInitialize (void);


	/**
		* This function gets the offset for listening to beacons from the network.
		*
		* \return the offset value
		*/
	uint32_t GetOffset ();

	/**
		* This function sets the offset for listening to beacons from the network.
		*
		* \param offset the offset value
		*/
	void SetOffset (uint32_t offset);

  /**
   * set the queue which is going to be used by this device
   *
   * @param queue
   */
  //virtual void SetQueue (Ptr<Queue> queue);


  /**
   * Notify the MAC that the PHY has finished a previously started transmission
   *
   */
  //void NotifyTransmissionEnd (Ptr<const Packet>);

  /**
   * Notify the MAC that the PHY has started a reception
   *
   */
  void NotifyReceptionStart ();


  /**
   * Notify the MAC that the PHY finished a reception with an error
   *
   */
  void NotifyReceptionEndError ();

  /**
   * Notify the MAC that the PHY finished a reception successfully
   *
   * @param p the received packet
   */
  void NotifyReceptionEndOk (Ptr<Packet> p, double snr);


  /**
   * This class doesn't talk directly with the underlying channel (a
   * dedicated PHY class is expected to do it), however the NetDevice
   * specification features a GetChannel() method. This method here
   * is therefore provide to allow LoRaNetDevice::GetChannel() to have
   * something meaningful to return.
   *
   * @param c the underlying channel
   */
  //void SetChannel (Ptr<Channel> c);


  /**
   * set the callback used to instruct the lower layer to start a TX
   *
   * @param c
   */
  //void SetGenericPhyTxStartCallback (GenericPhyTxStartCallback c);



  /**
   * Set the Phy object which is attached to this device.
   * This object is needed so that we can set/get attributes and
   * connect to trace sources of the PHY from the net device.
   *
   * @param phy the Phy object attached to the device.  Note that the
   * API between the PHY and the above (this NetDevice which also
   * implements the MAC) is implemented entirely by
   * callbacks, so we do not require that the PHY inherits by any
   * specific class.
   */
  //void SetPhy (Ptr<LoRaPhy> phy);

  /**
   * @return a reference to the PHY object embedded in this NetDevice.
   */
  //Ptr<LoRaPhy> GetPhy () const;


  // inherited from NetDevice
  virtual void DoDispose (void);
//virtual void SetIfIndex (const uint32_t index);
//virtual uint32_t GetIfIndex (void) const;
//virtual Ptr<Channel> GetChannel (void) const;
//virtual bool SetMtu (const uint16_t mtu);
//virtual uint16_t GetMtu (void) const;
//virtual void SetAddress (Address address);
//virtual Address GetAddress (void) const;
//virtual bool IsLinkUp (void) const;
//virtual void AddLinkChangeCallback (Callback<void> callback);
//virtual bool IsBroadcast (void) const;
//virtual Address GetBroadcast (void) const;
//virtual bool IsMulticast (void) const;
//virtual bool IsPointToPoint (void) const;
//virtual bool IsBridge (void) const;
//virtual bool Send (Ptr<Packet> packet, const Address& dest);
//virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest,
                         uint16_t protocolNumber);
//virtual Ptr<Node> GetNode (void) const;
//virtual void SetNode (Ptr<Node> node);
//virtual bool NeedsArp (void) const;
//virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
//virtual Address GetMulticast (Ipv4Address addr) const;
//virtual Address GetMulticast (Ipv6Address addr) const;
//virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
//virtual bool SupportsSendFrom (void) const;
  
	void SetReset (bool reset);

private:
	uint32_t m_offset; //!< The offset to listen to the beacons
	

 	EventId m_nextBeacon; //!< event of the next beacon
 	EventId m_beaconTimeout; //!< timeout of the current beacon
  double m_rssiBeacon [16]; //!< list of booleans if channel is available
  EventId m_transmission; //!< When the transmission is planned in a asubframe
	std::list<std::tuple<uint8_t,uint8_t> > m_channelRssiValues; //!<< the rssi of the beacon on each channel 
	

  /**
   * start the transmission of a packet by contacting the PHY layer
   */
  void StartTransmission ();

	/**
		* Execute this function if the beacon has not been received
		*/
	void BeaconTimeout (void);
	
	/**
		* Prepare the reception of an acknowledgment
		*
		* \param bandwidthSetting the bandwidth used in the coming reception
		* \param frequency the frequency used in the following reception	
		* \param spreadingfactor the spreading factor used by the acknowledgment
		*/
	virtual void DoPrepareReception (uint32_t bandwidthSetting, uint32_t frequency, uint32_t spreadingfactor);
  
	/** 
		* Put the transceiver in transceive state to receive the beacon;
		*/
	void ReceiveBeacon ();

  /**
   * Reception slot 1. The receiver checks whether there is a packet arriving. 
   */
	virtual void DoCheckReception ();

  /**
   * Reception slot 2. The receiver checks whether there is a packet arriving. 
   */
  virtual void DoCheckReception2 ();

	void SchedulePacket (std::list<std::tuple <uint8_t,uint8_t> > channels, double rssi);
	
protected:
	Time m_interBeacon; //time between beacons
	
	/**
		* This function returns the channel number when a frequency is given. It's the other direction as normally is done
		*
		* \param frequency the frequency you want to know the channel of
		* \return the channel number
		*/
	uint8_t GetChannelNbFromFrequency(uint32_t frequency); 
  
  /**
   *  This method tries to resend an unacknowledged packet. If the number of retransmissions is too high, it takes a new packet.
	 * 	If the last packet has arrived, a new packet from the queue is taken. If there are no packets at all, it doesn't do anything.
   */
	virtual void DoTryAgain();
};


} // namespace ns3

#endif /* LORA_RS_NET_DEVICE_H */
