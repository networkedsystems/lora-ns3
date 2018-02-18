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

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "lora-network-application.h"
#include "lora-network.h"

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("LoRaNetworkApplication");

	NS_OBJECT_ENSURE_REGISTERED (LoRaNetworkApplication);

	TypeId
	LoRaNetworkApplication::GetTypeId (void)
	{
  	static TypeId tid = TypeId ("ns3::LoRaNetworkApplication")
    	.SetParent<Application> ();
  	return tid;
	}

	// \brief LoRaNetworkApplication Destructor
	LoRaNetworkApplication::~LoRaNetworkApplication()
	{
		NS_LOG_FUNCTION (this);
	}

	void
		LoRaNetworkApplication::DoDispose (void)
		{
			NS_LOG_FUNCTION (this);
			m_network = 0;
		}

	void
		LoRaNetworkApplication::DoInitialize (void)
		{
			Application::DoInitialize ();
		}

	void
		LoRaNetworkApplication::SetNetwork (Ptr<LoRaNetwork> network)
		{
			m_network = network;
		}

	Ptr<LoRaNetwork> 
		LoRaNetworkApplication::GetNetwork (void)
		{
			return m_network;
		}

} // namespace ns3


