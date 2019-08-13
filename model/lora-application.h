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

#ifndef LORA_APPLICATION_H
#define LORA_APPLICATION_H

#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/node.h>
#include <ns3/callback.h>
#include <ns3/application.h>
#include <ns3/random-variable-stream.h>

namespace ns3 {

	class Node;
	class Socket;

	/**
	 * \ingroup lora
	 * \brief LoRa default application.
	 *
	 * This application generates empty packets every X seconds.  This application mimics a sensor. 
	 */
	class LoRaApplication : public Application
	{
		public:
			/**
			 * \brief Get the type ID.
			 * \return the object TypeId
			 */
			static TypeId GetTypeId (void);
			LoRaApplication ();
			virtual ~LoRaApplication ();

		private:
			/**
			 * This function is the function to schedule a sensing event. With every sensing event, we generate the packet.
			 */
			void Sense (void);
			void Send (const Ptr<Packet> packet);


			EventId m_SenseEvent;     //!< The event that will fire at m_stopTime to end the application
			Ptr<Socket> m_socket;		//!< The socket for this application. It sends directly to the netdevice (no routing!)
			uint8_t m_dataSize;		//!< The size of an empty message
			uint8_t m_port;			//!< The port to use (this is equal to the port MAC field in LoRa)
			Time m_interPacketTime;	//!< The time between two packets
			Ptr<RandomVariableStream> m_rand;
			bool m_random;
		protected:
			virtual void DoDispose (void);
			virtual void DoInitialize (void);
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

#endif /* APPLICATION_H */
