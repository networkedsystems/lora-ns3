/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#ifndef LORA_NETWORK_H
#define LORA_NETWORK_H


#include <ns3/traced-callback.h>
#include <string>
#include <stdint.h>
#include "ns3/node.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include <ns3/packet.h>
#include "ns3/net-device.h"
#include "ns3/random-variable-stream.h"
#include "ns3/application.h"

#include <map>
#include <iostream>
#include <string>
#include <cctype>
#include <list>

namespace ns3 {

class Channel;
class Socket;
class Packet;
class NetDevice;
/**
 * \ingroup lora
 *
 * \brief Global control interface of LoRaNetwork
 *
 * This class gives a network control interface that controls all the gateways in a controlled zone. 
 * It is assumed that there are no delays, from the gateways to the network. It should be noted that the minimal response time is greater than or
 * equal to one, so if only considering performance of the network, the delay the network should be neglible.
 * For the moment this class can only whitelist devices, but in the future, algorithms will be implemented in this class to create on overall optimal network.
 * This should be implemented as a net-device layer, but because nodes can't share information, this class is a subclass of Object.
 */
class LoRaNetwork : public Application
{
public:

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  LoRaNetwork();
  virtual ~LoRaNetwork();

	/**
		* \return true if we can transmit this message
		* \param packet the packet including header that needs to be transmitted
		*
		* Send a message (probably to one of the nodes in the network.) 
		* The message is forwarded as is to one of the gateways for transmission. 
		* Notice that a LoRaMacHeader is assumed in the beginning of this packet for knowing the destination.
		*/
  bool Send (Ptr<const Packet> packet) ;
	

  /**
   * \return true if and only if there is no ack send by any other gateway.
   * \param packet packet that is received at a LoRaNetDeviceGW.
   * \param protocol protocol that is used. 
   * \param address address of sending device.
   * 
   * This function connects to receivecallback of a LoRaNetDeviceGw. This could be multiple.
   */
  bool MessageReceived (Ptr<const Packet> packet, const Address& from) ;

  /**
   * \param address address of device which send the packet
   *
   * Removes a received messages from the waiting list assuming that there is no other gateway will receive
   * this message. The standard MAC does not allow messages to be send in the period between message and ACK, 
   * so 1 second should be enough. 
   */
  void RemoveMessage (const Address& address);

	/**
		* Whitelist a device in this network. The end-node with the given address will be controlled from this network, but the data will be used as well.
		* 
		* \param address address to whitelist.
		*/
  void WhiteListDevice (const Address& address);

	/**
		* Do dispose this object
		*/
	virtual void DoDispose (void);

private:
	uint8_t m_port; //!< port for this application to listen to. This allows gateways to connect to this network.
	Ptr<Socket> m_socket; //!< socket for the application 
	Ptr<NormalRandomVariable> m_random; //!< random variable
  std::vector <Address> justSend; //!<list of the latest received messages to prevent responding to to many messages
	std::list<std::tuple<Address, double, uint8_t,uint8_t,uint16_t> > m_RSSI; //!< The RSSI of all whitelisted nodes
	std::list<std::tuple<Address, uint32_t> > m_latest; //!<the latest frame number received for each device 
	std::list<std::tuple<Address, uint16_t, uint8_t, uint8_t> > m_settings; //!< list to keep track of the settings of each node
	std::vector <Address> m_whiteList; //!< list of all the whitelisted addresses
	std::map <Address, Ptr<Packet> > m_packetToTransmit; //!< Map of the messages to send to each address

	// Callback functions
	/**
		* The callback to notify the listeners that a messages has been arrived at the gateway.
		* This function shows only filtered messages. I.e. no duplicates
		*/
	TracedCallback<Ptr<const Packet> > m_netRxTrace;
	/**
		* The callback to notify the listeners that a messages has been arrived at the gateway.
		* This callback shows all messages
	 */
	TracedCallback<Ptr<const Packet> > m_netPromiscRxTrace;
  
	/**
		* This function handles the received messages from the socket. These message come from any of the base stations connected to this network.
		*
		* \param socket The socket that got the message.
		*/
	void HandleRead (Ptr<Socket> socket);

	/**
		* SendACK sends a message to a gateway with the message to transmit or not to 
		*
		* \param gateway gateway address to send the message from
		* \param sensor sensor address to send the message to
		*/
  void SendAck (const Address& gateway, const Address& sensor);
	
	/**
		* This function checks whether the given address is whitelisted in this network.
		* 
		* \param address the address to check
		* \return true if and only if the address is in the list of whitelisted devices
		*/
  bool IsWhiteListed (const Address& address);

protected:
  /**
   * \brief Application specific startup code
   *
   * The StartApplication method is called at the start time specified by Start
   * This method should be overridden by all or most application
   * subclasses.
   */
  void StartApplication (void);

  /**
   * \brief Application specific shutdown code
   *
   * The StopApplication method is called at the stop time specified by Stop
   * This method should be overridden by all or most application
   * subclasses.
   */
  void StopApplication (void);
};

} // namespace ns3

#endif /* LORA_NETWORK_H */
