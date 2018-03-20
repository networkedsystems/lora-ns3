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

#ifndef LORA_NETWORK_APPLICATION_H
#define LORA_NETWORK_APPLICATION_H

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/callback.h"
#include "ns3/application.h"
#include "lora-network.h"


namespace ns3 {

	class Node;
	class Packet;
	class LoRaMacCommand;

	/**
	 * \ingroup lora
	 * \brief The base class for all LoRa network applications
	 *
	 * This class is the base class for all applications installed on the network server. 
	 * It allows applications to interact with the underlying LoRa Network. 
	 * All messages sended towards the network should include a LoRaMacHeader.
	 */
	class LoRaNetworkApplication : public Application
	{
		public:
			/**
			 * \brief Get the type ID.
			 * \return the object TypeId
			 */
			static TypeId GetTypeId (void);
			virtual ~LoRaNetworkApplication ();

			/**
			 *\brief Newpacket rececives new packet and parses it
			 * This function should always be written.
			 * This function should be hooked to the callback interfaces of network
			 *
			 * \param packet new packet from the network.
			 */
			virtual void NewPacket (Ptr<const Packet> packet)=0;
			virtual void SetMacAnswer (Ptr<LoRaMacCommand> command) {};

			/**
			 * Setter for the network
			 *
			 * \param network the network this application is depending on. 
			 */
			void SetNetwork (Ptr<LoRaNetwork> network);

			/**
			 * Getter for the networ
			 *
			 * \return a pointer to the network attached to this network application
			 */
			Ptr<LoRaNetwork> GetNetwork (void);

			/**
			 * \param address of the node that has transmitted the MAC command
			 * The following functions are needed for the mac command: LinkAdrAns
			 * They do not need to be implemented except if needed for one or the other reason.
			 *
			 * ConfirmPower confirms the power setting of a node that was addressed with a LinkAdrReq command
			 */
			virtual void ConfirmPower (const Address& address){};

			/**
			 * \param address of the node that has transmitted the MAC command
			 * The following functions are needed for the mac command: LinkAdrAns
			 * They do not need to be implemented except if needed for one or the other reason.
			 *
			 * ConfirmDataRate confirms the data rate setting of a node that was addressed with a LinkAdrReq command
			 */
			virtual void ConfirmDataRate (const Address& address){};

			/**
			 * \param address of the node that has transmitted the MAC command
			 * The following functions are needed for the mac command: LinkAdrAns
			 * They do not need to be implemented except if needed for one or the other reason.
			 *
			 * ConfirmChannelMask confirms the channel mask setting of a node that was addressed with a LinkAdrReq command
			 */
			virtual void ConfirmChannelMask (const Address& address){};



		protected:
			virtual void DoDispose (void);
			virtual void DoInitialize (void);

			// the network application that is installed on the node. 
			Ptr<LoRaNetwork> m_network;

	};

} // namespace ns3

#endif /* APPLICATION_H */
