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
#include "lora-error-model.h"
#include <ns3/log.h>

#include <cmath>

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE("LoRaErrorModel");
	NS_OBJECT_ENSURE_REGISTERED (LoRaErrorModel);

	TypeId
		LoRaErrorModel::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::LoRaErrorModel")
				.SetParent<Object> ()
				.AddConstructor<LoRaErrorModel> ()
				;
			return tid;
		}

	LoRaErrorModel::LoRaErrorModel (void)
	{

	}

	long double 
		LoRaErrorModel::GetBER (double snr, uint16_t spreading, int bandwidth) const
		{
			//based on matlab model
			int bitrate = bandwidth*spreading/pow(2.0,spreading);
			// unused functions
			//long double z = sqrtl(snr*bandwidth/(long double)bitrate*spreading/2);
			//long double z = (long double)log((long double)spreading)/(long double)log(12)/2*(long double)(snr*2)*(long double)bandwidth/(long double)bitrate;
			//long double ber = pow(2.0,spreading)/4*erfcl(z);
			//long double ber = 0.5*erfcl(z);

			 long double z = 1.28*sqrt((long double)spreading*(long double)snr/(long double)bitrate*(long double)bandwidth-sqrt((long double)spreading)*1.28+0.4);
			 long double ber = 0.24*erfcl(z/sqrt(2));

			 if(snr > 0)
			 {
				 if (ber<1)
				 {
					 return ber;
				 }
				 else
				 {
					 return 1;
				 }
			 }
			 else
			 {
				 return -1;
			 }
		}

} // namespace ns3
