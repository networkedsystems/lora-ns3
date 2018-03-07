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


#ifndef NEW_CHANNEL_ANS_H
#define NEW_CHANNEL_ANS_H

#include <ns3/lora-mac-command.h>
namespace ns3 {
class LoRaNetDevice;

/*
 * \ingroup lora
 * Represent the Mac Command with the Frame Control and Sequence Number fields
 */
class NewChannelAns : public LoRaMacCommand
{

public:

  NewChannelAns (void);
  NewChannelAns (bool datarateOk, bool freqOk);

  ~NewChannelAns (void);

  std::string GetName (void) const;
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Execute(Ptr<LoRaNetworkApplication> netDevice,Address address);

	void SetDatarateOk (bool drOk);
	bool GetDatarateOk ();
	
	void SetFreqOk (bool freqOk);
	bool GetFreqOk ();

private:
	bool m_datarateOk;
	bool m_freqOk;

}; //NewChannelAns

}; // namespace ns-3

#endif /* NEW_CHANNEL_ANS_H*/
