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
 * Author: Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#ifndef RX_PARAM_SETUP_ANS_H
#define RX_PARAM_SETUP_ANS_H

#include <ns3/lora-mac-command.h>
namespace ns3 {

/*
 * \ingroup lora
 * Represent the Mac Command with the Frame Control and Sequence Number fields
 */
class RxParamSetupAns : public LoRaMacCommand
{

public:

  RxParamSetupAns (void);
  RxParamSetupAns (bool offset, bool dr, bool channel);

  ~RxParamSetupAns (void);

  std::string GetName (void) const;
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Execute(Ptr<LoRaNetworkApplication> netDevice,Address address);

	void SetOffsetAck (bool ack);
	bool GetOffsetAck ();
	void SetDrAck (bool ack);
	bool GetDrAck ();
	void SetChannelAck (bool ack);
	bool GetChannelAck ();

private:
	bool m_offsetAck;
	bool m_drAck;
	bool m_channelAck;

}; //RxParamSetupAns

}; // namespace ns-3

#endif /* RX_PARAM_SETUP_ANS_H*/
