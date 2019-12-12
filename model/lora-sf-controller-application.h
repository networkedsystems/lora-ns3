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

#ifndef LORA_SF_CONTROLLER_APPLICATION_H
#define LORA_SF_CONTROLLER_APPLICATION_H

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/callback.h"
#include "ns3/lora-network-application.h"
#include "sf-mab.h"

namespace ns3 {

class Node;
class Packet;

/**
 */
struct TableValue {Address addr; uint32_t lastValue[3]; uint32_t received[3]; uint32_t lastPacketNumber; double rssi;};
struct Setting {uint8_t sfs[3]; uint8_t acked;
};

/**
* \brief The base class for all ns3 applications
*
*/
class LoRaSfControllerApplication : public LoRaNetworkApplication
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  LoRaSfControllerApplication ();
  virtual ~LoRaSfControllerApplication ();


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
	virtual void ConfirmDataRate (const Address& address);

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
	//void NewRssi (double rssi, const Address& address);


protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);
private:
	static bool CheckTuple (const TableValue& first, const TableValue& second);
	uint8_t GetMinDr (uint8_t sfs);
	uint8_t GetMaxDr (uint8_t sfs);
	uint8_t GetChannelIndexFromFrequency(uint32_t frequency);
	void CalculateSetting (void);
	void SaveSetting(const Address& address, uint8_t spreadingFactors[3]);
	EventId m_settingCalculation;
	std::map<Address, TableValue> m_data;
	std::map<Address, Setting> m_settings;
	SfMab * m_bandits[3][10] = {{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}};
  uint32_t m_freqs[3] = {8681000,8683000,8685000};
	uint8_t m_lastChannel = 0;
};

} // namespace ns3

#endif /* LORA_SF_CONTROLLER_APPLICATION_H */
