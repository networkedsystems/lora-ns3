/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009, 2010 CTTC
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


#ifndef LORA_PHY_HEADER_H
#define LORA_PHY_HEADER_H

#include <ns3/header.h>

namespace ns3 {

	/**
	 * \ingroup lora
	 *
	 */
	class LoRaPhyHeader : public Header
	{
		public:

			/* 
			 * Constructor of PHY header
			 */
		LoRaPhyHeader (void);
		~LoRaPhyHeader (void);
		/* 
		 * Get TypeId
		 *
		 * \return TypeId of LoRaPHYHeader
		 */
		static TypeId GetTypeId (void);
		/*
		 * GetTypeId
		 *
		 * \return TypeId of LoRaPHYHeader
		 */
		virtual TypeId GetInstanceTypeId (void) const;
		/* 
		 * Returns the size if the header would be serialized
		 *
		 * \return size of the header in bytes
		 */
		virtual uint32_t GetSerializedSize (void) const;
		/* 
		 * Adds the header in the given buffer 
		 *
		 * \param start start of the packet
		 */
		virtual void Serialize (Buffer::Iterator start) const;
		/* 
		 * Deserializes the packet, and sets all the parameters from the buffer.
		 *
		 * \param start of the buffer where the bits are located.
		 * \return length of deserialized information.
		 */
		virtual uint32_t Deserialize (Buffer::Iterator start);
		/* 
		 * Prints the the information of this header to the given stream. 
		 *
		 * \param os the stream to print the information to.
		 */
		virtual void Print (std::ostream &os) const;
		/* 
		 * Get the length of the preamble
		 * 
		 * \return length of the preamble in bytes
		 */
		uint16_t GetPreamble (void) const;
		/* 
		 * Sets this header to be a beacon
		 */
		void SetBeacon ();
		/* 
		 * Sets this header to be a frame that contains data
		 */
		void SetData ();
		/* 
		 * Checks whether this header is the header of a beaconing frame
		 * 
		 * \return true if and only if header is a beacon
		 */
		bool IsBeacon ();
		/* 
		 * Checks whether this header is the header of a data frame
		 * 
		 * \return true if and only if this is a data header
		 */
		bool IsData (); 
		/* 
		 * Set the length of the preamble in bytes.
		 *
		 * \param preamble the length of the preamble, this should be bigger than 8
		 */
		void SetPreamble (uint16_t preamble);

		private:
		uint16_t m_preamble;
		bool m_beacon;
	};



} // namespace ns3

#endif /* LORA_PHY_HEADER_H */
