/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef LORA_SPECTRUM_SIGNAL_PARAMETERS_H
#define LORA_SPECTRUM_SIGNAL_PARAMETERS_H


#include <ns3/spectrum-signal-parameters.h>
#include <ns3/packet.h>
namespace ns3 {


	/**
	 * \ingroup lora 
	 *
	 * Signal parameters for LoRa.
	 */
	struct LoRaSpectrumSignalParameters : public SpectrumSignalParameters
	{

		public:
			// inherited from SpectrumSignalParameters
			virtual Ptr<SpectrumSignalParameters> Copy (void);

			/**
			 * default constructor
			 */
			LoRaSpectrumSignalParameters (void);
			~LoRaSpectrumSignalParameters (void);

			/**
			 * copy constructor
			 */
			LoRaSpectrumSignalParameters (const LoRaSpectrumSignalParameters& p);

			/**
			 * The packet being transmitted with this signal
			 */
			Ptr<Packet> packet;

			/**
			 * Setter for channel
			 *
			 * \param channel the frequency of this signal
			 */
			void SetChannel (uint32_t channel);

			/**
			 * Getter for channel
			 *
			 * \return the frequency of this signal
			 */
			uint32_t GetChannel (void);

			/**
			 * Setter for spreadingfactor 
			 *
			 * \param spreading the spreading factor used.
			 */
			void SetSpreading (uint16_t spreading);

			/**
			 * Getter for spreadingfactor 
			 *
			 * \return the spreading factor of this signal
			 */
			uint16_t GetSpreading ();

			/**
			 * Getter for bandwidth
			 *
			 * \return the bandwidth of thsi siganl
			 */
			uint32_t GetBandwidth ();

			/**
			 * Setter for bandwidth
			 *
			 * \param
			 */
			void SetBandwidth (uint32_t bw);

			/**
			 * Getter for number of bit errors 
			 *
			 * \return The number of bit errors
			 */
			uint16_t GetBer ();

			/**
			 * Setter for number of bit errors 
			 *
			 * \param ber the new amount of bit errors
			 */
			void SetBer (uint32_t ber); 

		private:
			// settings needed for receiver 
			// This is possible because GW can receive multiple signals at once.
			uint32_t m_channel; //!< the frequency this message is transmitted on (divided by 100)
			uint32_t m_bandwidth; //!< the bandwidth of this signal
			uint16_t m_spreading; //!< the spreading factor of this signal
			uint32_t m_ber; //!< the number of wrong bit in this packet
	};

}  // namespace ns3


#endif /* LORA_SPECTRUM_SIGNAL_PARAMETERS_H */
