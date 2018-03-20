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
 * Author: kwong yin <kwong-sang.yin@boeing.com>
 *				Brecht Reynders
 */

#include "lora-mac-header.h"
#include <ns3/address-utils.h>
#include "lora-mac-command.h"
#include <ns3/log.h>

#include <list>
#include <tuple>
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoRaMacHeader);
NS_LOG_COMPONENT_DEFINE ("LoRaMacHeader");


LoRaMacHeader::LoRaMacHeader ()
{
	NS_LOG_FUNCTION (this);
  SetType (LORA_MAC_CONFIRMED_DATA_UP);     // Assume Data frame
  SetNoFrmPend ();               // No Frame Pending
  SetNoAck ();                // No Ack Frame will be expected from recepient
  SetFrmCtrlRes (0);       
  SetFrameVer (0);          
  SetPort (0);
	SetNoAdrAck();
}


LoRaMacHeader::LoRaMacHeader (enum LoRaMacType loraMacType,
                                  uint16_t seqNum)
{
	NS_LOG_FUNCTION (this << loraMacType << seqNum);
  SetType (loraMacType);
  SetNoFrmPend ();               // No Frame Pending
  SetNoAck ();                // No Ack Frame will be expected from recepient
  SetFrmCtrlRes (0);        
  SetFrameVer (0);           
  SetPort (0);
  SetFrmCounter(seqNum);
	SetNoAdrAck();
}


LoRaMacHeader::~LoRaMacHeader ()
{
	NS_LOG_FUNCTION (this);
	m_commands.clear();
	m_channels.clear();
}


enum LoRaMacHeader::LoRaMacType
LoRaMacHeader::GetType (void) const
{
	NS_LOG_FUNCTION (this);
  switch (m_fctrlFrmType)
    {
    case 0:
      return LORA_MAC_JOIN_REQUEST; 
      break;
    case 1:
      return LORA_MAC_JOIN_ACCEPT;
      break;
    case 2:
      return LORA_MAC_UNCONFIRMED_DATA_UP;
      break;
    case 3:
      return LORA_MAC_UNCONFIRMED_DATA_DOWN;
      break;
    case 4:
      return LORA_MAC_CONFIRMED_DATA_UP;
      break;
    case 5:
      return LORA_MAC_CONFIRMED_DATA_DOWN;
      break;
    case 6:
      return LORA_MAC_BEACON;
      break;
    default:
      return LORA_MAC_PROPRIETARY;
    }
}

uint8_t
LoRaMacHeader::GetMacHeader (void) const
{
	NS_LOG_FUNCTION (this);
  uint8_t val = 0;
  val = (m_fctrlFrmType << 5) & (0x07 << 5);
  val |= (m_fctrlFrmVer) & (0x03);
  return val;
}  


uint8_t
LoRaMacHeader::GetFrameControl (void) const
{
	NS_LOG_FUNCTION (this);
  uint16_t val = 0;

  val = (m_fctrlAdr << 7) & (0x01 << 7);                 // Bit 0-2
  val |= (m_fctrlAdrAckReq << 6) & (0x01 << 6);         // Bit 3
  val |= (m_fctrlAck << 5) & (0x01 << 5);   // Bit 4
  val |= (m_fctrlFrmPending << 4) & (0x01 << 4);        // Bit 5
  val |= (GetCommandsLength()) & (0x0F);    // Bit 6
  return val;

}

bool
LoRaMacHeader::IsAdaptiveSupported (void) const
{
	NS_LOG_FUNCTION (this);
  return m_fctrlAdr;
}

bool
LoRaMacHeader::IsFrmPend (void) const
{
	NS_LOG_FUNCTION (this);
  return (m_fctrlFrmPending == 1);
}

bool
LoRaMacHeader::IsNoFrmPend (void) const
{
	NS_LOG_FUNCTION (this);
  return (m_fctrlFrmPending == 0);
}

bool
LoRaMacHeader::IsAck (void) const
{
	NS_LOG_FUNCTION (this);
  return (m_fctrlAck == 1);
}

bool
LoRaMacHeader::NeedsAck (void) const
{
	NS_LOG_FUNCTION (this);
	return (m_fctrlAdrAckReq || (m_fctrlFrmType == LORA_MAC_CONFIRMED_DATA_UP) || (m_fctrlFrmType == LORA_MAC_CONFIRMED_DATA_DOWN));
}

bool
LoRaMacHeader::IsAdrAck (void) const
{
	NS_LOG_FUNCTION (this);
  return m_fctrlAdrAckReq;
}

bool
LoRaMacHeader::IsNoAck (void) const
{
	NS_LOG_FUNCTION (this);
  return (m_fctrlAck == 0);
}

uint8_t
LoRaMacHeader::GetFrameVer (void) const
{
	NS_LOG_FUNCTION (this);
  return m_fctrlFrmVer;
}

Mac32Address
LoRaMacHeader::GetAddr (void) const
{
	NS_LOG_FUNCTION (this);
  return(m_addr);
}

bool
LoRaMacHeader::IsBeacon (void) const
{
	NS_LOG_FUNCTION (this);
	return (m_fctrlFrmType == LORA_MAC_BEACON);
}

bool
LoRaMacHeader::IsData (void) const
{
	NS_LOG_FUNCTION (this);
  return(m_fctrlFrmType == LORA_MAC_CONFIRMED_DATA_UP || m_fctrlFrmType == LORA_MAC_CONFIRMED_DATA_DOWN || m_fctrlFrmType == LORA_MAC_UNCONFIRMED_DATA_UP  || m_fctrlFrmType == LORA_MAC_UNCONFIRMED_DATA_DOWN);
}

bool
LoRaMacHeader::IsAcknowledgment (void) const
{
	NS_LOG_FUNCTION (this);
  return ((m_fctrlAck == 1) || (m_fctrlFrmType == LORA_MAC_CONFIRMED_DATA_DOWN) || (m_fctrlFrmType == LORA_MAC_UNCONFIRMED_DATA_DOWN));
}

bool
LoRaMacHeader::IsCommand (void) const
{
	NS_LOG_FUNCTION (this);
  return(m_port == 0);
}


LoRaMacCommandDirection
LoRaMacHeader::GetDirection ( void )
{
	NS_LOG_FUNCTION (this);
	if (m_fctrlFrmType == LORA_MAC_CONFIRMED_DATA_DOWN || m_fctrlFrmType == LORA_MAC_UNCONFIRMED_DATA_DOWN || m_fctrlFrmType == LORA_MAC_BEACON || m_fctrlFrmType == LORA_MAC_JOIN_ACCEPT)
		return FROMBASE;
	else
		return TOBASE;
}

void
LoRaMacHeader::SetType (LoRaMacType loraMacType)
{
	NS_LOG_FUNCTION (this << loraMacType);
  m_fctrlFrmType = loraMacType;
}

void 
LoRaMacHeader::SetMacHeader (uint8_t macHeader)
{
	NS_LOG_FUNCTION (this << (uint32_t) macHeader);
  m_fctrlFrmVer = (macHeader) & (0x03);             // Bit 0-2
  m_fctrlFrmType = (macHeader >> 5) & (0x07);         // Bit 5
}


void
LoRaMacHeader::SetFrameControl (uint8_t frameControl)
{
	NS_LOG_FUNCTION (this << (uint32_t) frameControl);
  m_fctrlCommandsLength = (frameControl) & (0x0F);             // Bit 0-2
  m_fctrlFrmPending = (frameControl >> 4) & (0x01);           // Bit 3
  m_fctrlAck = (frameControl >> 5) & (0x01);     // Bit 4
  m_fctrlAdrAckReq = (frameControl >> 6) & (0x01);         // Bit 5
  m_fctrlAdr = (frameControl >> 7) & (0x01);

}

void
LoRaMacHeader::SetAdaptive (void)
{
	NS_LOG_FUNCTION (this);
  m_fctrlAdr = 1;
}

void
LoRaMacHeader::SetNotAdaptive (void)
{
	NS_LOG_FUNCTION (this);
  m_fctrlAdr = 0;
}

void
LoRaMacHeader::SetAdrAck (void)
{
	NS_LOG_FUNCTION (this);
  m_fctrlAdrAckReq = 1;
}

void
LoRaMacHeader::SetNoAdrAck (void)
{
	NS_LOG_FUNCTION (this);
  m_fctrlAdrAckReq = 0;
}

void
LoRaMacHeader::SetFrmPend (void)
{
	NS_LOG_FUNCTION (this);
  m_fctrlFrmPending = 1;
}

void
LoRaMacHeader::SetNoFrmPend (void)
{
	NS_LOG_FUNCTION (this);
  m_fctrlFrmPending = 0;
}

void
LoRaMacHeader::SetAck (void)
{
	NS_LOG_FUNCTION (this);
  m_fctrlAck = 1;
}

void
LoRaMacHeader::SetNoAck (void)
{
	NS_LOG_FUNCTION (this);
  m_fctrlAck = 0;
}

void
LoRaMacHeader::SetFrameVer (uint8_t ver)
{
	NS_LOG_FUNCTION (this << (uint32_t) ver);
  m_fctrlFrmVer = ver;
}

void
LoRaMacHeader::SetAddr ( Mac32Address addr)
{
	NS_LOG_FUNCTION (this << addr);
  m_addr = addr;
}

void
LoRaMacHeader::SetFrmCounter (uint16_t frmCntr)
{
	NS_LOG_FUNCTION (this << frmCntr);
  m_auxFrmCntr = frmCntr;
}

uint16_t
LoRaMacHeader::GetFrmCounter () const
{
	NS_LOG_FUNCTION (this);
  return m_auxFrmCntr;
}

void
LoRaMacHeader::SetFrmCtrlRes (uint8_t cntr)
{
	NS_LOG_FUNCTION (this << (uint32_t) cntr);
  m_auxFrmCntr = cntr;
}

void
LoRaMacHeader::SetPort (uint8_t port)
{
	NS_LOG_FUNCTION (this << (uint32_t) port);
  m_port = port;
}

uint8_t 
LoRaMacHeader::GetPort (void) const
{
	NS_LOG_FUNCTION (this);
  return m_port;
}

std::string
LoRaMacHeader::GetName (void) const
{
	NS_LOG_FUNCTION (this);
  return "LoRa MAC Header";
}

TypeId
LoRaMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaMacHeader")
    .SetParent<Header> ()
    .AddConstructor<LoRaMacHeader> ();
  return tid;
}


TypeId
LoRaMacHeader::GetInstanceTypeId (void) const
{
	NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
LoRaMacHeader::PrintFrameControl (std::ostream &os) const
{
	NS_LOG_FUNCTION (this);
  os << "Frame Type = " << (uint32_t) m_fctrlFrmType  
     << ", Frame Pending = " << (uint32_t) m_fctrlFrmPending;

}

void
LoRaMacHeader::Print (std::ostream &os) const
{
	NS_LOG_FUNCTION (this);
  PrintFrameControl (os);

  os << ", Addr = " << m_addr;



//    for (std::list<LoRaMacCommand*>::const_iterator it = commands.begin();it!=commands.end();it++) {
//
//      os << "Lenght = " << (uint32_t) it->length
//         << ", Type = " << (uint32_t) it->type
//         << ", ID = " << (uint32_t)  it->id;

//      os << ", Data: ";
//      for (std::vector<uint8_t>::const_iterator it2 = it->content.begin();it2!=it->content.end();it2++) {
//        os << static_cast<uint32_t>(*it2);
//      }
//      os << "; ";
      
//    }
}

uint32_t
LoRaMacHeader::GetSerializedSize (void) const
{
	NS_LOG_FUNCTION (this);

	if (IsBeacon ())
		return m_channels.size()*2 + 10 + GetCommandsLength ();
  return 9 + GetCommandsLength ();
}


void
LoRaMacHeader::Serialize (Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;
  uint8_t frameControl = GetFrameControl ();
  i.WriteU8 (GetMacHeader());
  WriteTo (i, m_addr);
  i.WriteU8 (frameControl);
  i.WriteU16(m_auxFrmCntr);

	//Maybe add these at the back? then old devices can also read it. Now they will see weird mac commands.. :) 
	if(IsBeacon())
	{
		i.WriteU8(m_channels.size());
  	for (std::list<std::tuple<uint8_t,uint8_t> >::const_iterator it = m_channels.begin() ; it != m_channels.end() ; it++) {
    	i.WriteU8(std::get<0>(*it));
    	i.WriteU8(std::get<1>(*it));
  	}
	}
  for (std::list<Ptr<LoRaMacCommand>>::const_iterator it = m_commands.begin() ; it != m_commands.end() ; it++) {
    (*it) -> Serialize (i);
		i.Next((*it)->GetSerializedSize());
  }
	// This value should not be written when there are MAC commands.
	// To many iftests to implement this.
	i.WriteU8 (m_port);
}


uint32_t
LoRaMacHeader::Deserialize (Buffer::Iterator start)
{
	NS_LOG_FUNCTION (this);

  Buffer::Iterator i = start;
  SetMacHeader (i.ReadU8 ());
	
  ReadFrom (i, m_addr);
  uint8_t frameControl = i.ReadU8 ();
  SetFrameControl (frameControl);
  SetFrmCounter (i.ReadU16());

	if(IsBeacon())
	{
		uint8_t channelNb = i.ReadU8();
		for (uint8_t j = 0; j<channelNb; j++)
		{
			uint8_t rssi = i.ReadU8();
			uint8_t sf = i.ReadU8();
			AddChannel(rssi,sf);
		}
	}


	uint8_t counter = 0;

	while(counter< m_fctrlCommandsLength)
	{
		uint8_t cid = i.PeekU8 ();
		Ptr<LoRaMacCommand> command = LoRaMacCommand::CommandBasedOnCid(cid, GetDirection());
		counter+=command->Deserialize(i);
		SetMacCommand(command);
		i.Next(command->GetSerializedSize());
	}
	m_port = i.ReadU8 ();
	return i.GetDistanceFrom (start);
	//return 24;
}

uint8_t 
LoRaMacHeader::GetCommandsLength (void) const
{
	NS_LOG_FUNCTION (this);
    uint8_t size=0;
  	for (std::list<Ptr<LoRaMacCommand>>::const_iterator it = m_commands.begin() ; it != m_commands.end() ; it++) {
			size += (*it)->GetSerializedSize();
    }
		return size;
}

bool
LoRaMacHeader::SetMacCommand(Ptr<LoRaMacCommand> command)
{
	NS_LOG_FUNCTION (this << command->GetType ());
	if (GetCommandsLength () + command->GetSerializedSize () < 16)
	{
  	m_commands.push_back(command);
		return true;
	}
	return false;
}

std::list<Ptr<LoRaMacCommand> >
LoRaMacHeader::GetCommandList (void) 
{
	NS_LOG_FUNCTION (this);
  return m_commands;
}

void LoRaMacHeader::AddChannel (uint8_t rssi, uint8_t sf)
{
	NS_LOG_FUNCTION (this << (uint32_t) rssi << (uint32_t) sf);
	m_channels.push_back(std::make_tuple(rssi,sf));
}

std::list<std::tuple<uint8_t,uint8_t> > LoRaMacHeader::GetChannels ()
{
	NS_LOG_FUNCTION (this);
	return m_channels;
}
	
	void
LoRaMacHeader::Merge (LoRaMacHeader header)
{
	// check if compatible
	NS_LOG_FUNCTION(this << header.GetAddr());
	if (GetDirection () == header.GetDirection() && header.GetAddr () == GetAddr ())
	{
		if (GetDirection () == FROMBASE)
		{
			if (header.GetType() == LORA_MAC_CONFIRMED_DATA_DOWN)
				m_fctrlFrmType = LORA_MAC_CONFIRMED_DATA_DOWN;
		}
		else
			if (header.GetType() == LORA_MAC_CONFIRMED_DATA_UP)
				m_fctrlFrmType = LORA_MAC_CONFIRMED_DATA_UP;

		// merge list of mac commands (assume it is unique, otherwise, device might answer both of them)
		std::list <Ptr<LoRaMacCommand>> commands = header.GetCommandList();
		for (std::list <Ptr<LoRaMacCommand >>::const_iterator it = commands.begin (); it != commands.end(); it++)
		{
			SetMacCommand (*it);
		}
		if (m_port == 0)
			SetPort (header.GetPort ());
	}
}

// ----------------------------------------------------------------------------------------------------------


} //namespace ns3

