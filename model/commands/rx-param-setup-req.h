/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 The Boeing Company
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
#ifndef RX_PARAM_SETUP_REQ_H
#define RX_PARAM_SETUP_REQ_H

#include <ns3/lora-mac-command.h>
namespace ns3 {
class LoRaNetDevice;

/*
 * \ingroup lora
 * Represent the Mac Command with the Frame Control and Sequence Number fields
 */
class RxParamSetupReq : public LoRaMacCommand
{

public:

  RxParamSetupReq (void);
	RxParamSetupReq (uint8_t rx1Offset, uint8_t rx2Dr, uint32_t rx2Freq);

  ~RxParamSetupReq (void);

  std::string GetName (void) const;
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
	void Execute(Ptr<LoRaNetDevice> netDevice,Address address);
	
	/**
	 * Gets the offset of the first reception slot
	 *
	 * \return the offset
	 */
	uint8_t GetRx1Offset ();
	/**
	 * Sets the offset of the first reception slot
	 *
	 * \param rx1Offset the offset to use
	 */
	void SetRx1Offset (uint8_t rx1Offset);
	/**
	 * Gets the frequency of the second reception slot
	 *
	 * \return the frequency / 100
	 */
	uint32_t GetRx2Freq ();
	/**
	 * Sets the frequency to use in the second receive slot
	 *
	 * \param rx2Freq the frequency to use [divided by 100]
	 */
	void SetRx2Freq (uint32_t rx2Freq);
	/**
	 * Gets the data rate of the second reception slot
	 *
	 * \return the data rate
	 */
	uint8_t GetRx2Dr ();
	/**
	 * Sets the data rate of the second reception slot
	 *
	 * \param rx2Dr the datarate to use
	 */
	void SetRx2Dr (uint8_t rx2Dr);

private:
	uint8_t m_rx1Offset; //!< Offset to use for the first receive slot
	uint32_t m_rx2Freq; //!< frequency to use in the second receive slot
	uint8_t m_rx2Dr; //!< datarate to use in the second receive slot
}; //RxParamSetupReq

}; // namespace ns-3

#endif /* RX_PARAM_SETUP_REQ_H*/
