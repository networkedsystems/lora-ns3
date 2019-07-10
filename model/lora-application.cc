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
 * Author: Brecht Reynders<brecht.reynders@esat.kuleuven.be>
 */

// Implementation for ns3 Application base class.
// George F. Riley, Georgia Tech, Fall 2006

#include "lora-application.h"
#include "lora-net-device.h"
#include <ns3/mac32-address.h>
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/packet-socket.h"
#include "ns3/packet-socket-address.h"
#include <ns3/random-variable-stream.h>
#include <ns3/double.h>
#include <ns3/boolean.h>

namespace ns3 {


	NS_LOG_COMPONENT_DEFINE ("LoRaApplication");

	NS_OBJECT_ENSURE_REGISTERED (LoRaApplication);

	// Application Methods

	TypeId 
		LoRaApplication::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaApplication")
				.SetParent<Application> ()
				.SetGroupName("LoRa")
				.AddAttribute ("InterPacketTime", "Time between each burst of data",
						TimeValue (Seconds (600.0)),
						MakeTimeAccessor (&LoRaApplication::m_interPacketTime),
						MakeTimeChecker ())
				.AddAttribute ("DataSize", "Data size of each packet",
						UintegerValue (50),
						MakeUintegerAccessor (&LoRaApplication::m_dataSize),
						MakeUintegerChecker<uint8_t> ())
				.AddAttribute ("Port", "Port number to be used",
						UintegerValue (0),
						MakeUintegerAccessor (&LoRaApplication::m_port),
						MakeUintegerChecker<uint8_t> ())
				.AddAttribute ("RandomSend","Whether we should schedule it without randomness or with randomness",
						BooleanValue (false),
						MakeBooleanAccessor (&LoRaApplication::m_random),
						MakeBooleanChecker ())
				;
			return tid;
		}

	// \brief Application Constructor
	LoRaApplication::LoRaApplication()
	{
		NS_LOG_FUNCTION (this);
		m_socket = 0;
	}

	// \brief LoRaApplication Destructor
	LoRaApplication::~LoRaApplication()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaApplication::DoDispose (void)
		{
			NS_LOG_FUNCTION (this);
			m_socket = 0;
		}

	void
		LoRaApplication::DoInitialize (void)
		{
			Application::DoInitialize ();
			m_rand = CreateObject<UniformRandomVariable>();
			m_rand->SetAttribute("Max",DoubleValue(m_interPacketTime.GetSeconds()));
		}

	void LoRaApplication::StartApplication ()
	{ 
		NS_LOG_FUNCTION (this);
		// if there is no socket, generate one
		if (m_socket == 0)
		{
			m_socket = CreateObject<PacketSocket> ();
			StaticCast<PacketSocket>(m_socket)->SetNode (m_node);
			// We bind it directly to the netdevice
			m_socket->Bind ();
			PacketSocketAddress addr;
			addr.SetSingleDevice(false);
			// The output is always the gateway
			addr.SetPhysicalAddress (Mac32Address ("00:00:00:01"));
			m_socket->Connect (addr);
			m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
			m_socket->SetAllowBroadcast (false);
		}
		
		// Start sensing as a sensor
		m_SenseEvent = Simulator::ScheduleNow (&LoRaApplication::Sense, this);
	}

	void LoRaApplication::StopApplication ()
	{ 
		NS_LOG_FUNCTION (this);
		// Stop sensing
		m_SenseEvent.Cancel();
	}

	void LoRaApplication::Sense (void)
	{
		NS_LOG_FUNCTION (this);
		// Create a sensor reading of some size ...
		Ptr<Packet> packet = Create<Packet> (m_dataSize);
		// ... and send it.
		if (m_random)
			Simulator::Schedule(Seconds(m_rand->GetValue()),&LoRaApplication::Send,this,packet);
		else
			m_socket->Send (packet,0);

		// Schedule a new event
		m_SenseEvent = Simulator::Schedule(m_interPacketTime,&LoRaApplication::Sense,this);
	}

	void LoRaApplication::Send (const Ptr<Packet> packet)
	{
		m_socket->Send (packet,0);
	}

} // namespace ns3


