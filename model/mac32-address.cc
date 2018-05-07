/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
 * Author: Brecht Reynders<brecht.reynders@esat.kuleuven.be>
 */
#include "mac32-address.h"
#include "ns3/address.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include <iomanip>
#include <iostream>
#include <cstring>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Mac32Address");

ATTRIBUTE_HELPER_CPP (Mac32Address);

#define ASCII_a (0x41)
#define ASCII_z (0x5a)
#define ASCII_A (0x61)
#define ASCII_Z (0x7a)
#define ASCII_COLON (0x3a)
#define ASCII_ZERO (0x30)

/**
 * Converts a char to lower case.
 * \param c the char
 * \returns the lower case
 */
static char
AsciiToLowCase (char c)
{
  NS_LOG_FUNCTION (c);
  if (c >= ASCII_a && c <= ASCII_z) {
      return c;
    } else if (c >= ASCII_A && c <= ASCII_Z) {
      return c + (ASCII_a - ASCII_A);
    } else {
      return c;
    }
}


Mac32Address::Mac32Address ()
{
  NS_LOG_FUNCTION (this);
  std::memset (m_address, 0, 4);
}
Mac32Address::Mac32Address (const char *str)
{
  NS_LOG_FUNCTION (this << str);
  int i = 0;
  while (*str != 0 && i < 4) 
    {
      uint8_t byte = 0;
      while (*str != ASCII_COLON && *str != 0) 
        {
          byte <<= 4;
          char low = AsciiToLowCase (*str);
          if (low >= ASCII_a)
            {
              byte |= low - ASCII_a + 10;
            }
          else
            {
              byte |= low - ASCII_ZERO;
            }
          str++;
        }
      m_address[i] = byte;
      i++;
      if (*str == 0) 
        {
          break;
        }
      str++;
    }
  NS_ASSERT (i == 4);
}
void 
Mac32Address::CopyFrom (const uint8_t buffer[4])
{
  NS_LOG_FUNCTION (this << &buffer);
  std::memcpy (m_address, buffer, 4);
}
void 
Mac32Address::CopyTo (uint8_t buffer[4]) const
{
  NS_LOG_FUNCTION (this << &buffer);
  std::memcpy (buffer, m_address, 4);
}

bool 
Mac32Address::IsMatchingType (const Address &address)
{
  NS_LOG_FUNCTION (&address);  
  return address.CheckCompatible (GetType (), 4);
}
Mac32Address::operator Address () const
{
  return ConvertTo ();
}
Address 
Mac32Address::ConvertTo (void) const
{
  NS_LOG_FUNCTION (this);
  return Address (GetType (), m_address, 4);
}
Mac32Address 
Mac32Address::ConvertFrom (const Address &address)
{
  NS_LOG_FUNCTION (&address);
  NS_ASSERT (address.CheckCompatible (GetType (), 4));
  Mac32Address retval;
  address.CopyTo (retval.m_address);
  return retval;
}
Mac32Address 
Mac32Address::Allocate (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static uint64_t id = 0;
  id++;
  Mac32Address address;
  address.m_address[0] = (id >> 24) & 0xff;
  address.m_address[1] = (id >> 16) & 0xff;
  address.m_address[2] = (id >> 8) & 0xff;
  address.m_address[3] = (id >> 0) & 0xff;
  return address;
}
uint8_t 
Mac32Address::GetType (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static uint8_t type = Address::Register ();
  return type;
}

bool
Mac32Address::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return *this == GetBroadcast ();
}
bool 
Mac32Address::IsGroup (void) const
{
  NS_LOG_FUNCTION (this);
  return (m_address[0] & 0x01) == 0x01;
}
Mac32Address
Mac32Address::GetBroadcast (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static Mac32Address broadcast = Mac32Address ("ff:ff:ff:ff");
  return broadcast;
}
Mac32Address 
Mac32Address::GetMulticastPrefix (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static Mac32Address multicast = Mac32Address ("01:00:5e:00");
  return multicast;
}
Mac32Address
Mac32Address::GetMulticast6Prefix (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static Mac32Address multicast = Mac32Address ("33:33:00:00");
  return multicast;
}
uint32_t
Mac32Address::GetUInt()
{
  uint32_t temp = m_address[3] | ((m_address[2]) << 8) | ((m_address[1]) << 16) | ((m_address[0]) << 24) ;
  return temp;
}
Mac32Address 
Mac32Address::GetMulticast (Ipv4Address multicastGroup)
{
  NS_LOG_FUNCTION (multicastGroup);
  Mac32Address etherAddr = Mac32Address::GetMulticastPrefix ();
  //
  // We now have the multicast address in an abstract 32-bit container.  We 
  // need to pull it out so we can play with it.  When we're done, we have the 
  // high order bits in etherBuffer[0], etc.
  //
  uint8_t etherBuffer[4];
  etherAddr.CopyTo (etherBuffer);

  //
  // Now we need to pull the raw bits out of the Ipv4 destination address.
  //
  uint8_t ipBuffer[4];
  multicastGroup.Serialize (ipBuffer);

  //
  // RFC 1112 says that an Ipv4 host group address is mapped to an EUI-32
  // multicast address by placing the low-order 23-bits of the IP address into 
  // the low-order 23 bits of the Ethernet multicast address 
  // 01-00-5E-00-00-00 (hex). 
  //
  etherBuffer[3] |= ipBuffer[1] & 0x7f;

  //
  // Now, etherBuffer has the desired ethernet multicast address.  We have to
  // suck these bits back into the Mac32Address,
  //
  Mac32Address result;
  result.CopyFrom (etherBuffer);
  return result;
}
Mac32Address Mac32Address::GetMulticast (Ipv6Address addr)
{
  NS_LOG_FUNCTION (addr);
  Mac32Address etherAddr = Mac32Address::GetMulticast6Prefix ();
  uint8_t etherBuffer[4];
  uint8_t ipBuffer[16];

  /* a MAC multicast IPv6 address is like 33:33 and the four low bytes */
  /* for 2001:db8::2fff:fe11:ac10 => 33:33:FE:11:AC:10 */
  etherAddr.CopyTo (etherBuffer);
  addr.Serialize (ipBuffer);

  etherBuffer[2] = ipBuffer[12];
  etherBuffer[3] = ipBuffer[13];
  etherBuffer[4] = ipBuffer[14];

  etherAddr.CopyFrom (etherBuffer);

  return etherAddr;
}

std::ostream& operator<< (std::ostream& os, const Mac32Address & address)
{
  uint8_t ad[4];
  address.CopyTo (ad);

  os.setf (std::ios::hex, std::ios::basefield);
  os.fill ('0');
  for (uint8_t i=0; i < 3; i++) 
    {
      os << std::setw (2) << (uint32_t)ad[i] << ":";
    }
  // Final byte not suffixed by ":"
  os << std::setw (2) << (uint32_t)ad[3];
  os.setf (std::ios::dec, std::ios::basefield);
  os.fill (' ');
  return os;
}

std::istream& operator>> (std::istream& is, Mac32Address & address)
{
  std::string v;
  is >> v;

  std::string::size_type col = 0;
  for (uint8_t i = 0; i < 4; ++i)
    {
      std::string tmp;
      std::string::size_type next;
      next = v.find (":", col);
      if (next == std::string::npos)
        {
          tmp = v.substr (col, v.size ()-col);
          address.m_address[i] = strtoul (tmp.c_str(), 0, 16);
          break;
        }
      else
        {
          tmp = v.substr (col, next-col);
          address.m_address[i] = strtoul (tmp.c_str(), 0, 16);
          col = next + 1;
        }
    }
  return is;
}


} // namespace ns3
