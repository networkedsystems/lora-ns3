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

#ifndef LORA_SINK_APPLICATION_H
#define LORA_SINK_APPLICATION_H

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/callback.h"
#include "ns3/application.h"

namespace ns3 {

class Node;
class Packet;
class Socket;

/**
	* \ingroup lora
* \brief The application running on LoRa gateways 
*
* This application is running on each LoRa Gateway. The fuctionality is really trivial: it forwards all incoming messages to the network application and forwards all messages from the network server to the nodes. 
*/
class LoRaSinkApplication : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

	/**
		* Default constructor
		*/
  LoRaSinkApplication ();

	/**
		* Default destructor
		*/
  virtual ~LoRaSinkApplication ();

	/**
		* HandleRead is the function that gets executed every time there is a message from the network server. 
		* This function reads all incoming messages and checks if it needs to transmit these on its network. 
		* Forwarding functionality is enabled or disabled with the ACK flag and a downlink message. If this flag is not set, the message is NOT transmitted
		*
		* \param socket The socket that received the messages
		*/
	void HandleRead (Ptr<Socket> socket);
	/**
		* HandleLoRa is the function that gets executed every time there is a new LoRa message received. 
		* All messages are forwarded to the network server and a downlink slot is scheduled for the case messages need to be transmitted on the network
		*
		* \param socket The socket that is listening on the LoRa Network.
		*/
	void HandleLoRa (Ptr<Socket> socket);

	/**
		* SetNetDevice stores the netdevice that is responsible for the LoRa Network.
		*
		* \param device Netdevice that is listening to the LoRa Network.
		*/
	void SetNetDevice (Ptr<NetDevice> device);

private:
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

	Ptr<Socket> m_socket; //!< socket for connecting with the LoRa network server
	Ptr<Socket> m_loraSocket; //!< socket for connecting with the LoRa network
	Address m_serverAddress; //!< address of the LoRa network server
	uint16_t m_port; //!< port of the LoRa Network server
	Ptr<NetDevice> m_device; //!< netdevice connected to the LoRa Network
	
protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

};

} // namespace ns3

#endif /* APPLICATION_H */
