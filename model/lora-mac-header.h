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
 *  Author: kwong yin <kwong-sang.yin@boeing.com>
 */

/**
 * the following classes implements the 802.15.4 Mac Header
 * There are 4 types of 802.15.4 Mac Headers Frames, and they have these fields
 *
 *    Headers Frames  : Fields
 *    -------------------------------------------------------------------------------------------
 *    Beacon          : Frame Control, Sequence Number, Address Fields+, Auxiliary Security Header++.
 *    Data            : Frame Control, Sequence Number, Address Fields++, Auxiliary Security Header++.
 *    Acknowledgment  : Frame Control, Sequence Number.
 *    Command         : Frame Control, Sequence Number, Address Fields++, Auxiliary Security Header++.
 *
 *    + - The Address fields in Beacon frame is made up of the Source PAN Id and address only and size
 *        is  4 or 8 octets whereas the other frames may contain the Destination PAN Id and address as
 *        well. (see specs).
 *    ++ - These fields are optional and of variable size
 */

#ifndef LORA_MAC_HEADER_H
#define LORA_MAC_HEADER_H

#include <ns3/header.h>
#include <ns3/mac32-address.h>
#include <ns3/lora-mac-command.h>

namespace ns3 {
/**
 * \brief Write an Mac32Address to a Buffer
 * \param i a reference to the buffer to write to
 * \param ad the Mac32address
 */
void WriteTo (Buffer::Iterator &i, Mac32Address ad);

/**
 * \brief Read a Mac32Address from a Buffer
 * \param i a reference to the buffer to read from
 * \param ad a reference to the Mac32Address to be read
 */
void ReadFrom (Buffer::Iterator &i, Mac32Address &ad);


/*
 * \ingroup lora
 * Represent the Mac Header with the Frame Control and Sequence Number fields
 */
class LoRaMacHeader : public Header
{

public:
  enum LoRaMacType
  {
    LORA_MAC_JOIN_REQUEST = 0, 
    LORA_MAC_JOIN_ACCEPT = 1,
    LORA_MAC_UNCONFIRMED_DATA_UP = 2,
    LORA_MAC_UNCONFIRMED_DATA_DOWN = 3,
    LORA_MAC_CONFIRMED_DATA_UP = 4,
    LORA_MAC_CONFIRMED_DATA_DOWN = 5,
    LORA_MAC_BEACON=6,
    LORA_MAC_PROPRIETARY
  };

  enum KeyIdModeType
  {
    IMPLICIT = 0,
    NOKEYSOURCE = 1,
    SHORTKEYSOURCE = 2,
    LONGKEYSOURCE = 3
  };

  enum FrameVersionType {
    LORA2003 = 0,
    LORA2006 = 1,
    FRAME_VERSION_RESERVED = 2
  };


  LoRaMacHeader (void);

  LoRaMacHeader (enum LoRaMacType loraMacType,      // Data, ACK, Control MAC Header must have
                   uint16_t seqNum);                     // frame control and sequence number.
                                                        // Beacon MAC Header must have frame control,
                                                        // sequence number, source PAN Id, source address.

  ~LoRaMacHeader (void);


  enum LoRaMacType GetType (void) const;
  uint8_t GetFrameControl (void) const;
  uint8_t GetMacHeader (void) const;
  bool IsAdaptiveSupported (void) const;
  bool IsFrmPend (void) const;
  bool IsNoFrmPend (void) const;
  bool IsAck (void) const;
  bool IsNoAck (void) const;
	bool NeedsAck (void) const;
  bool IsAdrAck (void) const;
  uint8_t GetFrmCtrlRes (void) const;
  uint8_t GetFrameVer (void) const;

  Mac32Address GetAddr (void) const;

  uint8_t GetPort (void) const;

  uint16_t GetFrmCounter (void) const;

  bool IsBeacon (void) const;
  bool IsData (void) const;
  bool IsAcknowledgment (void) const;
  bool IsCommand (void) const;


	void SetDirection (LoRaMacCommandDirection direction);
	LoRaMacCommandDirection GetDirection (void);
  void SetType (enum LoRaMacType loraMacType);
  void SetFrameControl (uint8_t frameControl);
  void SetMacHeader (uint8_t macHeader);
  void SetAdaptive (void);
  void SetNotAdaptive (void);
  void SetAdrAck (void);
  void SetNoAdrAck (void);
  void SetAck (void);
  void SetNoAck (void);
  void SetFrmPend (void);
  void SetNoFrmPend (void);
  void SetFrmCtrlRes (uint8_t res);
  void SetFrameVer (uint8_t ver);

  void SetSeqNum (uint8_t seqNum);

  /* The Source/Destination Addressing fields are only set if SrcAddrMode/DstAddrMode are set */
  void SetAddr ( Mac32Address addr);
  void SetPort ( uint8_t port ) ;

  void SetFrmCounter (uint16_t frmCntr);


  std::string GetName (void) const;
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  void PrintFrameControl (std::ostream &os) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  uint8_t GetCommandsLength (void) const;

  bool SetMacCommand(Ptr<LoRaMacCommand> command);
  std::list<Ptr<LoRaMacCommand> > GetCommandList (void);

	void AddChannel (uint8_t rssi, uint8_t sf);
	std::list<std::tuple<uint8_t,uint8_t> > GetChannels ();

	void Merge (LoRaMacHeader header);

private:
  /* Frame Control 2 Octets */
  /* Frame Control fields */
  uint8_t m_fctrlFrmType;               // Bit 0-2    = 0 - Beacon, 1 - Data, 2 - Ack, 3 - Command
  uint8_t m_fctrlFrmVer;                // Bit 12-13

  uint8_t m_fctrlAdr;
  uint8_t m_fctrlAdrAckReq;
  uint8_t m_fctrlAck;                   // Bit 5
  uint8_t m_fctrlFrmPending;            // Bit 4
	uint8_t m_fctrlCommandsLength;
 
  uint8_t m_port;
   
  /* Addressing fields */
  Mac32Address m_addr;        // 0 or 8 Octet


  uint16_t m_auxFrmCntr;


  std::list<Ptr<LoRaMacCommand> > m_commands; //IE Header List
  std::list<std::tuple<uint8_t,uint8_t> > m_channels; //IE Header List


}; //LoRaMacHeader

}; // namespace ns-3

#endif /* LORA_MAC_HEADER_H */
