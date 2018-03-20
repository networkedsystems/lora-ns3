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

#ifndef GW_TRAILER_H
#define GW_TRAILER_H

#include <ns3/trailer.h>

namespace ns3 {

	class Packet;

	/**
	 * \ingroup lora
	 * \brief This trailer adds the received signal strength at the end of the packet. 
	 *
	 * This is a dirty workaround to transmit extra information to the different fields. 
	 * In the long run, this trailer should be removed and should be replaced with a struct with more information.
	 */
	class GwTrailer : public Trailer
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
		GwTrailer (void);
		~GwTrailer (void);

		// Inherited from the Trailer class.
		virtual TypeId GetInstanceTypeId (void) const;
		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);

		/**
		 * Get the received power strength of the transmitted message
		 *
		 * \return the rssi value
		 */
		double GetRssi (void) const;

		/**
		 * attach the received power strength to this message
		 *
		 * \param p the rssi value for this packet 
		 */
		void SetRssi (double rssi);

		/**
			* Returns the gateway that received this message
			*
			* \return the node id of the node that received the message
			*/
		uint32_t GetGateway(void);
		
		/**
			* Set the gateway id of the receiver of this message
			*
			* \param gatewayId the node id of the receiving gateway
			*/
		void SetGateway (uint32_t gatewayId);

		private:
		/**
		 * The rssi value with this packet
		 */
		double m_rssi;

		uint32_t m_gatewayId; //!< the id of the receiving gateway
	};

} // namespace ns3

#endif /* GW_TRAILER_H */
