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
 * Author: Brecht Reynders <brecht.reynders@esat.kuleuven.be> 
 */
#ifndef MAC32_ADDRESS_H
#define MAC32_ADDRESS_H

#include <stdint.h>
#include <ostream>
#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

namespace ns3 {

class Address;

/**
 * \ingroup address
 * 
 * \brief an EUI-32 address
 *
 * This class can contain 32 bit IEEE addresses.
 *
 * \see attribute_Mac32Address
 */
class Mac32Address
{
public:
  Mac32Address ();
  /**
   * \param str a string representing the new Mac32Address
   *
   * The format of the string is "xx:xx:xx:xx:xx:xx"
   */
  Mac32Address (const char *str);

  /**
   * \param buffer address in network order
   *
   * Copy the input address to our internal buffer.
   */
  void CopyFrom (const uint8_t buffer[4]);
  /**
   * \param buffer address in network order
   *
   * Copy the internal address to the input buffer.
   */
  void CopyTo (uint8_t buffer[4]) const;

  /**
   * \returns a new Address instance
   *
   * Convert an instance of this class to a polymorphic Address instance.
   */
  operator Address () const;
  /**
   * \param address a polymorphic address
   * \returns a new Mac32Address from the polymorphic address
   * 
   * This function performs a type check and asserts if the
   * type of the input address is not compatible with an
   * Mac32Address.
   */
  static Mac32Address ConvertFrom (const Address &address);
  /**
   * \param address address to test
   * \returns true if the address matches, false otherwise.
   */
  static bool IsMatchingType (const Address &address);
  /**
   * Allocate a new Mac32Address.
   * \returns newly allocated mac32Address
   */
  static Mac32Address Allocate (void);

  /**
   * \returns true if this is a broadcast address, false otherwise.
   */
  bool IsBroadcast (void) const;

  /**
   * \returns true if the group bit is set, false otherwise.
   */
  bool IsGroup (void) const;

  /**
   * \returns the broadcast address
   */
  static Mac32Address GetBroadcast (void);

  /**
   * Get uint32_t representation of Mac32 address.
   */
  uint32_t GetUInt (void);

  /**
   * \param address base IPv4 address
   * \returns a multicast address
   */
  static Mac32Address GetMulticast (Ipv4Address address);

  /**
   * \brief Get multicast address from IPv6 address.
   * \param address base IPv6 address
   * \returns a multicast address
   */
  static Mac32Address GetMulticast (Ipv6Address address);

  /**
   * \returns the multicast prefix (01:00:5e:00).
   */
  static Mac32Address GetMulticastPrefix (void);

  /**
   * \brief Get the multicast prefix for IPv6 (33:33:00:00).
   * \returns a multicast address.
   */
  static Mac32Address GetMulticast6Prefix (void);

  /**
   * TracedCallback signature for Mac32Address
   *
   * \param [in] value Current value of the Mac32Address
   */
  typedef void (* TracedCallback)(const Mac32Address value);
  
private:
  /**
   * \returns a new Address instance
   *
   * Convert an instance of this class to a polymorphic Address instance.
   */
  Address ConvertTo (void) const;

  /**
   * \brief Return the Type of address.
   * \return type of address
   */
  static uint8_t GetType (void);

  /**
   * \brief Equal to operator.
   *
   * \param a the first operand
   * \param b the first operand
   * \returns true if the operands are equal
   */
  friend bool operator == (const Mac32Address &a, const Mac32Address &b);

  /**
   * \brief Not equal to operator.
   *
   * \param a the first operand
   * \param b the first operand
   * \returns true if the operands are not equal
   */
  friend bool operator != (const Mac32Address &a, const Mac32Address &b);

  /**
   * \brief Less than operator.
   *
   * \param a the first operand
   * \param b the first operand
   * \returns true if the operand a is less than operand b
   */
  friend bool operator < (const Mac32Address &a, const Mac32Address &b);

  /**
   * \brief Stream insertion operator.
   *
   * \param os the stream
   * \param address the address
   * \returns a reference to the stream
   */
  friend std::ostream& operator<< (std::ostream& os, const Mac32Address & address);

  /**
   * \brief Stream extraction operator.
   *
   * \param is the stream
   * \param address the address
   * \returns a reference to the stream
   */
  friend std::istream& operator>> (std::istream& is, Mac32Address & address);

  uint8_t m_address[4]; //!< address value
};

ATTRIBUTE_HELPER_HEADER (Mac32Address);

inline bool operator == (const Mac32Address &a, const Mac32Address &b)
{
  return memcmp (a.m_address, b.m_address, 4) == 0;
}
inline bool operator != (const Mac32Address &a, const Mac32Address &b)
{
  return memcmp (a.m_address, b.m_address, 4) != 0;
}
inline bool operator < (const Mac32Address &a, const Mac32Address &b)
{
  return memcmp (a.m_address, b.m_address, 4) < 0;
}

std::ostream& operator<< (std::ostream& os, const Mac32Address & address);
std::istream& operator>> (std::istream& is, Mac32Address & address);

} // namespace ns3

#endif /* MAC32_ADDRESS_H */
