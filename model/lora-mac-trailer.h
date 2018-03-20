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

#ifndef LORA_MAC_TRAILER_H
#define LORA_MAC_TRAILER_H

#include <ns3/trailer.h>

namespace ns3 {

	class Packet;

	/**
	 * \ingroup lora
	 * \brief The MAC Trailer of a LoRa packet
	 *
	 * This is the implementation of the MAC layer of a LoRa packet. Basically, this includes the MIC of the packet. 
	 */
	class LoRaMacTrailer : public Trailer
	{
		public:

		/**
		 * Get the type ID.
		 *
		 * \return the object TypeId
		 */
		static TypeId GetTypeId (void);

		/**
		 * Default constructor for a MAC trailer with disabled FCS calculation.
		 */
		LoRaMacTrailer (void);
		~LoRaMacTrailer (void);

		// Inherited from the Trailer class.
		virtual TypeId GetInstanceTypeId (void) const;
		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);

		/**
		 * Get this trailers FCS value. If FCS calculation is disabled for this
		 * trailer, the returned value is always 0.
		 *
		 * \return the FCS value.
		 */
		uint16_t GetMIC (void) const;

		/**
		 * Calculate and set the FCS value based on the given packet.
		 *
		 * \param p the packet for which the FCS should be calculated
		 */
		void SetMIC (Ptr<const Packet> p);

		/**
		 * Check the FCS of a given packet against the FCS value stored in the
		 * trailer. The packet itself should contain no trailer. If FCS calculation is
		 * disabled for this trailer, CheckMIC() will always return true.
		 *
		 * \param p the packet to be checked
		 * \return false, if the FCS values do not match, true otherwise
		 */
		bool CheckMIC (Ptr<const Packet> p);

		private:
		uint16_t m_MIC; //!< The FCS value stored in this trailer.
		static const uint16_t LORA_MIC_LENGTH; //!< The length in octets of the LORA MIC FCS field

	};

} // namespace ns3

#endif /* LORA_MAC_TRAILER_H */
