/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 *  Author: Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

/**
 * \brief A base class for any MAC command
 *
 * This class implements the default functionality of a LoRa MAC Command . This class is a base class and should be added to the LoRaMacHeader.
 */



#ifndef LORA_MAC_COMMAND_H
#define LORA_MAC_COMMAND_H

#include "ns3/buffer.h"
#include "ns3/object.h"
#include <ns3/header.h>
#include <stdint.h>
#include "lora-net-device.h"
#include "lora-network-application.h"

namespace ns3 {

	class Address;

	// Defining new comman CIDs (this list should be updated whenever there are new commands)
	typedef	enum {
		LINK_CHECK = 2, 
		LINK_ADR = 3,
		DUTY_CYCLE = 4,
		RX_PARAM_SETUP = 5,
		DEV_STATUS = 6,
		NEW_CHANNEL = 7,
		RX_TIMING = 8,
		LORA_COMMAND_PROPRIETARY
	} LoRaMacCommandCid;

	// This enum defines the direction of the command. 
	typedef enum {
		TOBASE = 0,
		FROMBASE = 1,
	} LoRaMacCommandDirection;



	class LoRaMacCommand : public Object
	{

		public:
			virtual ~LoRaMacCommand();
			LoRaMacCommandCid GetType (void) const;

			static LoRaMacCommandCid GetType (uint8_t);
			LoRaMacCommandDirection GetDirection (void) const;

			static TypeId GetTypeId (void);
			virtual TypeId GetInstanceTypeId (void) const;

			/**
				* \param cid the command identifier of this MAC command
				*
				* Set the command identifier of this command
				*/
			void SetType (LoRaMacCommandCid cid);

			/**
				* \param direction The requested direction.
				*
				* Set the direction of this mac command
				*/
			void SetDirection (LoRaMacCommandDirection direction); 

			/**
				* \param os The stream for printing
				*
				* Print prints out this MAC command
				*/
			void Print (std::ostream &os) const;
			// Classes inherited from Header and Packet
			virtual uint32_t GetSerializedSize (void) const =0;
			virtual void Serialize (Buffer::Iterator start) const = 0;
			virtual uint32_t Deserialize (Buffer::Iterator start) = 0;

			/**
				* \param netDevice this is the netdevice that will undergo the changes of the command
				* \param address this is the address to whom this message is addresed
				*
				* Execute executes the function of the given MAC command on a given netdevice or application
				*/
			virtual void Execute(Ptr<LoRaNetDevice> netDevice, Address address) {};

			/**
				* \param app This is the application that will undergo the changes of the command
				* \param address this is the address to whom this message is addresed
				*
				* Execute executes the function of the given MAC command on a given netdevice or application
				*/
			virtual void Execute(Ptr<LoRaNetworkApplication> app, Address address) {};

			/**
				* \return A pointer to a newly created LoRaMacCommand
				* \param cid The command identifier of wanted command
				* \param direction The direction of the the wanted command
				*
				* CommandBasedOnCid returns a pointer to a LoRaMacCommand with a specified cid and direction.
				* This could be seen as a constructor for different derived classes.
				*/
			static Ptr<LoRaMacCommand> CommandBasedOnCid (uint8_t cid, LoRaMacCommandDirection direction);

		protected:
			LoRaMacCommandCid m_cid; //!< The LoRa MAC command
			LoRaMacCommandDirection m_direction; //!< The direction of this MAC command
	}; //LoRaMacCommand

}; // namespace ns-3

#endif /* LORA_MAC_COMMAND_H*/
