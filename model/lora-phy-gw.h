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

#ifndef LORA_PHY_GW_H
#define LORA_PHY_GW_H

#include "lora-phy.h"
#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-phy.h>
#include "lora-error-model.h"
#include "lora-spectrum-signal-parameters.h"
#include <ns3/event-id.h>
#include <ns3/random-variable-stream.h>
#include <ns3/callback.h>
namespace ns3 {

	class SpectrumChannel;
	class SpectrumValue;
	class SpectrumModel;
	class NetDevice;
	struct SpectrumSignalParameters;

	/**
	 * \ingroup lora
	 *
	 * Physical layer implementation for base stations
	 *
	 */
	class LoRaPhyGw : public LoRaPhy 
	{

		public:
			LoRaPhyGw ();
			~LoRaPhyGw ();

			void DoDispose (void);

			static TypeId GetTypeId (void);

			/**
			 * get the associated NetDevice instance
			 *
			 * \return a Ptr to the associated NetDevice instance
			 */
		uint32_t GetReceptions ();
		/**
		 * Notify the SpectrumPhy instance of an incoming signal
		 *
		 * \param params the parameters of the signals being received
		 */
		void StartRx (Ptr<SpectrumSignalParameters> params);

		/**
		 * Verify if packet is correctly received. 
		 *
		 * \param params the parameters of the signals being received
		 */
		void EndRx (Ptr<LoRaSpectrumSignalParameters> params);
		/**
		 * Start the transmission of the given packet.
		 * 
		 * \params packet the packet to send
		 * \return true if and only if the packet has been send by the physical layer.
		 */
		bool StartTx(Ptr<Packet> packet); 
		/**
		 * Set Callback when reception ends
		 */
		void SetReceptionEndCallback(Callback<void,Ptr<Packet>,uint32_t,uint8_t,uint32_t,double> callback);
		/**
		 * Get the number of collisions that happened at receptions.
		 * 
		 * \return number of collisions during the simulation
		 */
		uint32_t GetCollisions (void); 



		private:
		uint32_t m_collisions; //!< Collisions that are happened
		std::vector <Ptr<LoRaSpectrumSignalParameters> > m_params; //!<parameters of all the arriving packets
		Callback<void, Ptr<Packet>,uint32_t, uint8_t, uint32_t,double> m_ReceptionEnd; //!<callbackfunction with extra field


		/**
		 * Update the BER for all receiving transmissions based on latest information 
		 */
		void UpdateBer (void);
	};
} // namespace ns3

#endif /* LORA_PHY_GW_H */
