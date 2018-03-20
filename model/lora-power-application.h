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

#ifndef LORA_POWER_APPLICATION_H
#define LORA_POWER_APPLICATION_H

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
class LoRaPowerApplication : public LoRaNetworkApplication
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  LoRaPowerApplication ();
  virtual ~LoRaPowerApplication ();


	/**
		*\brief Newpacket rececives new packet and parses it
		*/
	void NewPacket (Ptr<const Packet> packet);
	
	// LinkAdrAns
	/**
		* ConfirmPower saves the power setting when the device acknowledges this.
		*	
		*	\param	address	The address that acknowledged.
		*/
	virtual void ConfirmPower (const Address& address);

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

	/**
		*	NewRssi saves the new Rssi values in the data.
		*
		* \param	rssi	the new Rssi measurement
		*	\param	address	the address to which this measurement belongs
		*/
	void NewRssi (double rssi, const Address& address);
	/**
		*GetSetting returns the settings for the given address.
		*
		* \param address	the address of the setting of the device.
		*	\return	the settings of the requested device.
		*/
	std::tuple<uint16_t,uint8_t,uint8_t> GetSetting (const Address& address);
	/**
		* CheckTuple checks whether the received power of the first tuple is bigger than the received power of the second tuple.
		* 
		*	This function is used to sort the list based on received power.
		*
		*	\param	first	the first tuple to compare
		*	\param	second	the second tuple to compare
		*	\return	true if the power of the first is bigger than the received power of the second.
		*/
	static bool CheckTuple (const std::tuple<Address,double,uint8_t,uint8_t,uint16_t>& first, const std::tuple<Address,double,uint8_t,uint8_t,uint16_t>& second);

	void CalculateSetting (void);
	void CalculatePower (uint8_t groupIndex, std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator start, std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator end);
	std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator GetSpreading(std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator start,std::list<std::tuple<Address,double,uint8_t,uint8_t,uint16_t> >::iterator end);
	void SaveSetting(const Address& address, uint8_t power, uint8_t datarate, uint16_t channelMask);
	uint8_t m_second[3] = {0,0,0};
	std::list<std::tuple<Address,double, uint8_t,uint8_t,uint16_t> > m_RSSI;
	std::map<Address, std::tuple<uint16_t, uint8_t, uint8_t> > m_settings;


protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

};

} // namespace ns3

#endif /* APPLICATION_H */
