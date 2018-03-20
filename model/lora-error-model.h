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
#ifndef LORA_ERROR_MODEL_H
#define LORA_ERROR_MODEL_H

#include <ns3/object.h>

namespace ns3 {

	/**
	 * \ingroup lora
	 * \brief The bit error model for LoRa
	 *
	 * Model for the bit errors in noisy/AWGN channels for LoRa. Based on the paper published in SCVT 2017
	 */
	class LoRaErrorModel : public Object
	{
		public:
			/**
			 * Get the type ID.
			 *
			 * \return the object TypeId
			 */
			static TypeId GetTypeId (void);

			LoRaErrorModel (void);

			/**
			 * Return BER for given SNR.
			 *
			 * \return bit error rate
			 * \param snr SNR expressed as a power ratio (i.e. not in dB)
			 * \param spreading spreading factor used 
			 * \param bandwidth used in this lora configuration
			 */
			long double GetBER (double snr, uint16_t spreading,int bandwidth) const;


	};


} // namespace ns3

#endif /* LORA_ERROR_MODEL_H */


