/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010
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

#ifndef LORA_NET_DEVICE_H
#define LORA_NET_DEVICE_H

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
 * \brief The MAC implementation of the LoRa protocol based on the LoRaWAN 1.0 A class
 *
 * This devices implements the following features:
 *  - MAC addressing
 *  - Aloha MAC:
 *    + packets transmitted as soon as possible
 *    + a new packet is queued if previous one is still being transmitted
 *    + no acknowledgements, hence no retransmissions
 */
class LoRaNetDevice : public NetDevice
{
public:
	// the state variable
  enum State
  {
    IDLE, TIMEOUT, TX, RX, RETRANSMISSION, RX1_PENDING, RX2_PENDING, BEACON
  };

	/**
		* See also in higher classes
		*/
  static TypeId GetTypeId (void);

  LoRaNetDevice ();
  virtual ~LoRaNetDevice ();


  /**
   * set the queue which is going to be used by this device
   *
   * \param queue the wanted queue structure
   */
  virtual void SetQueue (Ptr<Queue> queue);


  /**
   * Notify the MAC that the PHY has finished a previously started transmission
   *
	 * \param the transmitted packet
   */
  void NotifyTransmissionEnd (Ptr<const Packet>);

  /**
   * Notify the MAC that the PHY has started a reception
   */
  void NotifyReceptionStart ();


  /**
   * Notify the MAC that the PHY finished a reception with an error
   */
  void NotifyReceptionEndError ();

  /**
   * Notify the MAC that the PHY finished a reception successfully
   *
   * \param p the received packet
	 * \param rssi the received signal strength of the received packet
   */
  void NotifyReceptionEndOk (Ptr<Packet> p, double rssi);


  /**
   * This class doesn't talk directly with the underlying channel (a
   * dedicated PHY class is expected to do it), however the NetDevice
   * specification features a GetChannel() method. This method here
   * is therefore provide to allow LoRaNetDevice::GetChannel() to have
   * something meaningful to return.
   *
   * \param c the underlying channel
   */
  void SetChannel (Ptr<Channel> c);


  /**
   * set the callback used to instruct the lower layer to start a TX
   *
   * \param c The generic start of phy message
   */
  void SetGenericPhyTxStartCallback (GenericPhyTxStartCallback c);



  /**
   * Set the Phy object which is attached to this device.
   * This object is needed so that we can set/get attributes and
   * connect to trace sources of the PHY from the net device.
   *
   * \param phy the Phy object attached to the device.  Note that the
   * API between the PHY and the above (this NetDevice which also
   * implements the MAC) is implemented entirely by
   * callbacks, so we do not require that the PHY inherits by any
   * specific class.
   */
  void SetPhy (Ptr<LoRaPhy> phy);

  /**
		* Get the physical layer corresponding to this MAC layer
		* 
   * \return a reference to the PHY object embedded in this NetDevice.
   */
  Ptr<LoRaPhy> GetPhy () const;


  // inherited from NetDevice
  virtual void DoDispose (void);
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, uint16_t protocolNumber);
  virtual bool Send (Ptr<Packet> packet, const Address& dest);
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest,
                         uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual Address GetMulticast (Ipv4Address addr) const;
  virtual Address GetMulticast (Ipv6Address addr) const;
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;
  
  /**
   * Set the given power manually.
	 *
	 * \param power the power to set
   */
  bool SetMaxPower (uint8_t power);

  /**
   * Set the maximal and minimal datarate manually.
	 *
	 * \param datarate the wanted datarate
	 * \return if the setting was succesfull
   */
  bool SetMaxDataRate (uint8_t datarate);
  /**
   * Set the maximal and minimal datarate manually.
	 *
	 * \param datarate the wanted datarate
	 * \param index the channel index for setting this datarate
   */
  bool SetMaxDataRate (uint8_t datarate, uint8_t index);
  /**
   * Set the maximal and minimal datarate manually.
	 *
	 * \param datarate the wanted datarate
   */
  bool SetMinDataRate (uint8_t datarate);
  /**
   * Set the maximal and minimal datarate manually.
	 *
	 * \param datarate the wanted datarate
	 * \param index the channel index for setting this datarate
   */
  bool SetMinDataRate (uint8_t datarate, uint8_t index);
  /**
   * Set the maximal and minimal datarate manually.
	 *
	 * \param datarate the wanted datarate
	 * \param index the channel index for setting this datarate
   */
  bool SetDelay (uint8_t delay);
 
	/**
		* SetReset resets the node after every message from the gateway to use the lowest spreading factor possible.
		* If this is not enabled, the nodes continue using the spreading factor they where able to communicate with the gateway
		*
		* \param reset whether or not the node should be reset
		*/
	void SetReset (bool reset);
	/**
   * Checks whether the receiver is the intended receiver, if the address is correct, receiveing continues, otherwise transceiver will be turned off
	 *
	 * \params header The MAC-header of the arriving packet
   */
  void CheckCorrectReceiver (LoRaMacHeader header);
  /**
   * Get the average retransmission ocunt before a packet arrives
	 * 
	 * \return the average number of retransmissions
   */
  double GetAvgRetransmissionCount();
  /**
   * Get the average waiting time before a packet arrives
	 * 
	 * \return the average waiting time in seconds
   */
  double GetAvgTime();

  /**
   * Get the number of packets that not got acknowledged.
	 * 
	 * \return number of packets that got lost
   */
  uint32_t GetMissed();

  /**
   * Get the number of packets that got acknowledged.
	 * 
	 * \return number of packets that got acknowledged
   */
  uint32_t GetArrived();

	/**
		* Set the amount of retransmissions for unacknowledged packets.
		*
		* \param repetitions amount of times the packet needs to be repeated
		*/
	void SetNbRep (uint8_t repetitions);

	/**
		* Set the channel mask for this device.
		*
		* \param channelMask 16 bit channel mask for each channel 1 bit
		* \return true if and only if the channel mask is accepted.
		*/
	bool SetChannelMask (uint16_t channelMask);

	/**
		* Sets an MAC command answer in the following packet.
		*
		* \param command the command to transmit in the next message
		*/
	void SetMacAnswer (Ptr<LoRaMacCommand> command);
  
  /**
   *  This method tries to resend an unacknowledged packet. If the number of retransmissions is too high, it takes a new packet.
	 * 	If the last packet has arrived, a new packet from the queue is taken. If there are no packets at all, it doesn't do anything.
   */
	void TryAgain();

	/**
		* Set the duty cycle
		*/
	void SetDutyCycle (uint8_t dutycycle);

	/**
		* Get the duty cycle
		*/
	uint8_t GetDutyCycle ();

	/**
		* SetRx2Settings sets the settings for the second receive slot
		* 
		* \param datarate the datarate of the second receive slot as specified in 7.1.3
		* \param freq frequency of the second receive slot
		*/
	bool SetRx2Settings(uint8_t datarate, uint32_t freq);
	
	/**
		* GetRx2Freq returns the frequency of the second receive slot
		*
		* \return the frequency
		*/
	uint32_t GetRx2Frequency ();
	
	/**
		* GetRx2Datarate returns the datarate of the second receive slot
		*
		* \return the datarate as in 7.1.3
		*/
	uint8_t GetRx2Datarate ();
	
	/**
		* SetDlOffset sets the offset for the first receive slot
		*
		* \param offset value that specifies the offset between 0 and 5
		*/
	bool SetDlOffset (uint8_t offset);
	
	/**
		* GetDlOffset returns the offset for the first receive slot
		*
		* \return the offset to be used in receive slot one
		*/
	uint8_t GetDlOffset ();

	
	/**
		* AddChannel adds a channel with the proposed parameters.
		*
		* \param index the index to set the new channel
		* \param freq the frequency of the new channel
		*/
	bool AddChannel (uint8_t index, uint32_t freq);
	/**
		* AddChannel adds a channel with the proposed parameters.
		*
		* \param index the index to set the new channel
		* \param freq the frequency of the new channel
		*/
	void RemoveChannel (uint8_t index);

protected:

  Ptr<Queue> m_queue; //!< queue for packets to send
  Ptr<Node>    m_node; //!< node of this netdevice
  Ptr<Channel> m_channel; //!< channel that is used
  Mac32Address m_address; //!< address of this device

  Ptr<UniformRandomVariable> m_random; //!< random generator
  uint32_t m_ifIndex; //!< indexnumber of the interface
  uint32_t m_mtu; //!< maximal amount of bytes to transmit
	uint8_t m_nbRep; //!< amount of repetitions for 1 packet in unacknowledged data.
	uint8_t m_rep; //!< actual amount of repetitions of the current packet.
  bool m_linkUp; //!< tells if the link is up
  State m_state; //!< state of the transceiver
  uint16_t m_channelIndex; //!< index to transmit on 
  bool channelAvailable [16] = {true,true,true,false,false,false,false,false,false,false,false,false,false,false,false,false}; //!< list of booleans if channel is available
  double LastSend [16]; //!< time in seconds with the last transmission;
  Ptr<Packet> m_currentPkt; //!< packet that is current being transmitted
  EventId m_event; //
  EventId m_freeChannel; //
  EventId m_event2; //
  uint8_t m_delay; //!< delay to wait for acknowledgement
  Ptr<LoRaPhy> m_phy; //!< physical layer of this device
  uint16_t m_seqNum; //!< current packet number
  uint8_t retransmissionCount; //!< number of retransmission of the current packet
  uint8_t m_dutyCycle; //!< maximal allowed dutycycle
  double m_waitingFactor; //!< factor that should be waited after the transmission of a packet (to speed up calculations)
  Time startTimePacket; //!< time that device tried to send a packet for the first time
  double averageTime; //!< average time to get an acknowledgement when starting to send a packet
  double avgRetransmissionCount; //!< average retransmission count
  uint32_t missedCount; //!< number of packets that did not get acknowledged
  uint32_t arrivedCount; //!< number of packets that arrived and got acknowledged
	uint8_t m_powerIndex; //!< power index to transmit with
	uint32_t m_rx2Freq; //!< Frequency of the second reception
	uint8_t m_rx2Datarate; //!< Settings of the second reception
	uint8_t m_rx1Offset; //!< Settings of the second reception
	bool m_reset; //!< reset to highest data rate after each message from the base station
	uint8_t m_ackCnt; //!< Amount of messages without an ACK
	uint8_t ADR_ACK_LIMIT = 64; //!< The limit when we have to have received a message from the base station.
	uint8_t spreading [16] = {12,11,10,9,8,7,7,1,1,1,1,1,1,1,1,1}; //!< list of spreadingfactors following the standard
  uint32_t frequencies [16] = {8681000,8683000,8685000,8681000,8683000,8685000,0,0,0,0,0,0,0,0,0,0}; //!< list of frequencies 
  uint16_t maxDatarate [16] = {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5}; //!< min datarate to use on each channel
  uint16_t minDatarate [16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //!< max datarate to use on each channel
  uint16_t datarate [16] = {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5}; //!< datarate to use on each channel
  double power [16] = {0,0.025,0.0126,0.0063,0.0031,0.0016,0,0,0,0,0,0,0,0,0,0}; //!< list of powers following the standard
  uint32_t bandwidth [16] = {125000,125000,125000,125000,125000,125000,250000,125000,125000,125000,125000,125000,125000,125000,125000,125000};//!<list of bandwidths
	std::list<Ptr<LoRaMacCommand>> m_answers; //!< List of answers to transmit to the other side
	
  /**
   * start the transmission of a packet by contacting the PHY layer
   */
  void StartTransmissionNoArgs ();
  
	/**
   *  This method tries to resend an unacknowledged packet. If the number of retransmissions is too high, it takes a new packet.
	 * 	If the last packet has arrived, a new packet from the queue is taken. If there are no packets at all, it doesn't do anything.
   */
	virtual void DoTryAgain();
  
  /**
   * This method searches a channel that is available for transmission.
	 *
	 * \return the index of the channel that can be used for the next transmission
   */
	uint8_t GetFreeChannel ();
  
  /**
   *  The backoff period for the given channel is over. It can be used again.
	 *  
	 *  \params channelIndex the index of the channel that can be used again.
   */
	void FreeChannel (uint8_t channelIndex);

	void PrepareReception (uint32_t bandwidth, uint32_t frequency, uint32_t spreading);
	virtual void DoPrepareReception (uint32_t bandwidth, uint32_t frequency, uint32_t spreading);
  
  /**
   * Reception slot 1. The receiver checks whether there is a packet arriving. 
   */
	void CheckReception ();
  /**
   * Reception slot 1. The receiver checks whether there is a packet arriving. 
	 * This is the actual implementation.
	 * But this can be overridden by other mac protocols
   */
	void DoCheckReception ();

  /**
   * Reception slot 2. The receiver checks whether there is a packet arriving. 
   */
  virtual void CheckReception2 ();

  /**
   * Reception slot 2. The receiver checks whether there is a packet arriving.
	 * This is the actual implementation.
   */
  virtual void DoCheckReception2 ();
	
  //traceback functions
  TracedCallback<Ptr<const Packet> > m_macTxTrace;
  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;
  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;
  TracedCallback<Ptr<const Packet> > m_macRxTrace;
  
	/**
   * List of callbacks to fire if the link changes state (up or down).
   */
  TracedCallback<> m_linkChangeCallbacks;
  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;
  GenericPhyTxStartCallback m_phyMacTxStartCallback;
	
	/**
		* Start the transmission of a packet with the given parameters
		*
		* \return true if message got accepted by physical layer
		* \param packet packet to transmit
		* \param frequency frequency to transmit on
		* \param datarate datarate to be used
		* \param powerIndex power to be used
		*/
	bool StartTransmission (Ptr<Packet> packet, uint32_t frequency, uint8_t datarate, uint8_t powerIndex);
	
	/**
		* GetRxDatarate provide the datarate for the first reception slot
		* 
		* \param dr the transmission data rate
		* \return the reception datarate
		*/
	uint8_t GetRxDatarate (uint8_t dr);
};


} // namespace ns3

#endif /* LORA_NET_DEVICE_H */
