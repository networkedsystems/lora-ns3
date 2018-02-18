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

#ifndef LORA_NO_POWER_APPLICATION_H
#define LORA_NO_POWER_APPLICATION_H

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/callback.h"
#include "ns3/lora-network-application.h"

namespace ns3 {

class Node;
class Packet;

/**
 */

/**
* \brief The base class for all ns3 applications
*
*/
class LoRaNoPowerApplication : public LoRaNetworkApplication
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  LoRaNoPowerApplication ();
  virtual ~LoRaNoPowerApplication ();


	/**
		*\brief Newpacket rececives new packet and parses it
		*/
	void NewPacket (Ptr<const Packet> packet);

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

};

} // namespace ns3

#endif /* APPLICATION_H */
