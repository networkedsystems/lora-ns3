/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation
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

#include "lora-sink-application.h"
#include "lora-mac-header.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/packet-socket.h"
#include "ns3/packet-socket-address.h"
#include "ns3/socket-factory.h"
#include "ns3/address-utils.h"
#include "ns3/udp-socket.h"
#include "gw-trailer.h"
#include "ns3/uinteger.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaSinkApplication");

	NS_OBJECT_ENSURE_REGISTERED (LoRaSinkApplication);

	// Application Methods

	TypeId 
		LoRaSinkApplication::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaSinkApplication")
				.SetParent<Application> ()
				.SetGroupName("lora")
				.AddConstructor<LoRaSinkApplication> ()
				.AddAttribute ("ServerAddress",
						"The address of the LoRaWAN server that controls everything.",
						AddressValue (),
						MakeAddressAccessor (&LoRaSinkApplication::m_serverAddress),
						MakeAddressChecker ())
				.AddAttribute ("Port",
						"The port on the remote server",
						UintegerValue(100),
						MakeUintegerAccessor (&LoRaSinkApplication::m_port),
						MakeUintegerChecker<uint16_t> ())
				;
			return tid;
		}

	// \brief Application Constructor
	LoRaSinkApplication::LoRaSinkApplication()
	{
		m_socket = 0;
		NS_LOG_FUNCTION (this);
	}

	// \brief LoRaSinkApplication Destructor
	LoRaSinkApplication::~LoRaSinkApplication()
	{
		NS_LOG_FUNCTION (this);
		m_socket = 0;
		m_loraSocket = 0;
	}

	void
		LoRaSinkApplication::DoDispose (void)
		{
			NS_LOG_FUNCTION (this);
		}

	void
		LoRaSinkApplication::DoInitialize (void)
		{
			// Create socket here, where we can send data on. 
			Application::DoInitialize ();
		}

	// Protected methods
	// StartApp and StopApp will likely be overridden by application subclasses
	void LoRaSinkApplication::StartApplication ()
	{ // Provide null functionality in case subclass is not interested
		NS_LOG_FUNCTION (this);  

		if (m_socket == 0)
		{
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			m_socket = Socket::CreateSocket (m_node, tid);
			if (Ipv4Address::IsMatchingType(m_serverAddress) == true)
			{
				m_socket->Bind ();
				m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_serverAddress), m_port));
			}
			else if (Ipv6Address::IsMatchingType(m_serverAddress) == true)
			{
				m_socket->Bind6 ();
				m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_serverAddress), m_port));
			}
		}
		m_socket->SetRecvCallback (MakeCallback(&LoRaSinkApplication::HandleRead,this));
		m_socket->SetAllowBroadcast (true);
		if (m_loraSocket == 0)
		{
			m_loraSocket = CreateObject<PacketSocket> ();
			StaticCast<PacketSocket>(m_loraSocket)->SetNode (m_node);
			m_loraSocket->BindToNetDevice (m_device);
			PacketSocketAddress ad; 
			ad.SetPhysicalAddress (m_device->GetAddress ());
			ad.SetSingleDevice (m_device->GetIfIndex ());
			ad.SetProtocol (0);
			m_loraSocket->Bind (ad);
			m_loraSocket->Listen ();
			m_loraSocket->SetRecvCallback (MakeCallback (&LoRaSinkApplication::HandleLoRa,this));
			m_loraSocket->SetAllowBroadcast (true);
		}

	}

	void LoRaSinkApplication::HandleLoRa (Ptr<Socket> socket)
	{
		NS_LOG_FUNCTION (this << socket);
		Ptr<Packet> pkt;
		while (pkt = socket->Recv ())
		{
			NS_LOG_DEBUG("Packet arrived");
			if (m_socket != 0 && pkt->GetSize () > 0)
			{
				Ptr<Packet> toBackend = pkt->Copy();
				toBackend->RemoveAllPacketTags ();
				toBackend->RemoveAllByteTags ();
				m_socket->Send(toBackend);
			}
		}
	}

	void LoRaSinkApplication::SetNetDevice (Ptr<NetDevice> device)
	{
		NS_LOG_FUNCTION (this << device);
		m_device = device;
	}

	void LoRaSinkApplication::HandleRead (Ptr<Socket> socket)
	{
		NS_LOG_FUNCTION (this << socket);
		Ptr<Packet> pkt;
		while (pkt = socket->Recv ())
		{
				Ptr<Packet> fromBackend = pkt->Copy();
				fromBackend->RemoveAllPacketTags ();
				fromBackend->RemoveAllByteTags ();
				LoRaMacHeader mac;
				fromBackend->PeekHeader (mac);
				PacketSocketAddress destination;
				destination.SetProtocol (0);
				destination.SetSingleDevice (false);
				destination.SetPhysicalAddress (mac.GetAddr ());
				m_loraSocket->SendTo (fromBackend,0,destination);
		}
	}

	void LoRaSinkApplication::StopApplication ()
	{ // Provide null functionality in case subclass is not interested
		NS_LOG_FUNCTION (this);
	}

} // namespace ns3


