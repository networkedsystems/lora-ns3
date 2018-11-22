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
#include "gw-trailer.h"
#include <ns3/log.h>
#include <ns3/packet.h>
#include <cmath>

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("GwTrailer");
	NS_OBJECT_ENSURE_REGISTERED (GwTrailer);


	GwTrailer::GwTrailer (void)
	{
	}

	GwTrailer::~GwTrailer (void)
	{
	}

	TypeId
		GwTrailer::GetTypeId (void)
		{
			static TypeId tid = TypeId ("ns3::GwTrailer")
				.SetParent<Trailer> ()
				.AddConstructor<GwTrailer> ()
				;
			return tid;
		}

	TypeId
		GwTrailer::GetInstanceTypeId (void) const
		{
			return GetTypeId ();
		}

	void
		GwTrailer::Print (std::ostream &os) const
		{
			os << " RSSI = " << m_rssi;
		}

	uint32_t
		GwTrailer::GetSerializedSize (void) const
		{
			return sizeof(double)+4+9;
		}

	void
		GwTrailer::Serialize (Buffer::Iterator start) const
		{
			start.Prev (sizeof(double)+4+9);
			uint8_t value[sizeof(double)];
			memcpy(value,&m_rssi,sizeof(double));
			//const uint8_t* value = reinterpret_cast<const uint8_t*>(&m_rssi);
			start.Write (value,sizeof(double));
			start.WriteU32 (m_gatewayId);
			start.WriteU8 (m_sf);
			start.WriteU32 (m_bandwidth);
			start.WriteU32 (m_frequency);
		}

	uint32_t
		GwTrailer::Deserialize (Buffer::Iterator start)
		{
			start.Prev (sizeof(double)+4+9);
			uint8_t value[sizeof(double)];
			start.Read (value,sizeof(double));
			memcpy(&m_rssi,value,sizeof(double));
			//m_rssi = *reinterpret_cast<double*>(value);
			m_gatewayId = start.ReadU32 ();
			m_sf = start.ReadU8 ();
			m_bandwidth= start.ReadU32 ();
			m_frequency = start.ReadU32 ();

			return sizeof(double)+4+9;
		}

	double
		GwTrailer::GetRssi (void) const
		{
			NS_LOG_FUNCTION(this << m_rssi);
			return m_rssi;
		}

	void
		GwTrailer::SetRssi (double rssi)
		{
			NS_LOG_FUNCTION(this << rssi << 10*std::log10(125000*rssi));
			m_rssi = rssi;
		}

	void 
		GwTrailer::SetGateway (uint32_t gatewayId)
		{
			NS_LOG_FUNCTION (this << gatewayId);
			m_gatewayId = gatewayId;
		}

	uint32_t 
		GwTrailer::GetGateway (void)
		{
			return m_gatewayId;
		}

	uint8_t 
		GwTrailer::GetSpreadingFactor()
		{
			return m_sf;
		}
	void GwTrailer::SetSpreadingFactor(uint8_t sf)
	{
		m_sf = sf;
	}

	uint32_t GwTrailer::GetBandwidth()
	{
		return m_bandwidth;
	}
	void GwTrailer::SetBandwidth(uint32_t bandwidth)
	{
		m_bandwidth = bandwidth;
	}
		
	uint32_t GwTrailer::GetFrequency()
	{
		return m_frequency;
	}
		void GwTrailer::SetFrequency (uint32_t freq)
		{
			m_frequency = freq;
		}

} //namespace ns3
