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

#include "sf-mab.h"
#include <iostream>
#include <experimental/random>

namespace ns3 {

	/**
	 *
	 *
	 */
	SfMab::SfMab():
		SfMab(1,5)
	{
	}

	SfMab::SfMab(uint32_t combinations,uint8_t spreadingfactors)
	{
		m_lastSelection = 0;
		m_beta = 0.25;
		m_epsilon = 1;
		m_combinations = combinations;
		m_sfs = spreadingfactors;
	  uint32_t arms = 0;
		for (uint8_t i = 0; i<combinations; i++)
			arms += m_sfs-i;
		m_values = new double[arms];
		for (uint32_t i = 0; i< arms; i++)
			m_values[i]=1;
		m_arms = arms;
	}

	SfMab::~SfMab()
	{
		delete [] m_values;
	}

		uint8_t
		SfMab::GetSpreadingFactors (uint32_t arm)
		{
			for(uint8_t i = 0; i< m_combinations; i++)
			{
				if ( arm+i < (uint32_t) (m_sfs +1))
				{
					return (0xFF>>(7-i))<<(arm-1);
				}
				else
				{
					arm -= m_sfs - i;
				}
			}

			return 255;
		}

	uint32_t
		SfMab::GetArm (uint8_t spreadingfactors)
		{
			uint32_t combinations_discount = 0;
			uint8_t spreadingfactorsDetected = 0;
			uint8_t offset = 1;
			uint32_t arm = 0;
			for (uint32_t i = 0; i < 8; i++)
			{
				if ((spreadingfactors>>i & 1) ==1)
				{
					arm += combinations_discount + (i+1)*offset;
					combinations_discount = m_sfs - spreadingfactorsDetected;
					spreadingfactorsDetected++;
					offset = 0;
				}
				else
				{
					if (combinations_discount > 0)
						return arm;
				}

			}
			return 0xFFFFFFFF;
		}


	/**
	 *\brief Newpacket rececives new packet and parses it
	 */
	void SfMab::NewObservation (double observation)
	{
		
		// combinatorial values

		uint32_t minArm = m_lastSelection+1; 
		uint32_t maxArm = m_lastSelection+1; 
		for (uint8_t i = 0; i<m_combinations; i++)
		{
			for (uint32_t armi = minArm; armi <= std::min((uint32_t)m_arms,maxArm); armi++)
			{
				// default behaviour MAB for i == 0
				UpdateValue(m_beta/(i+1), observation, armi);
			}
			uint8_t minSf = GetSpreadingFactors(minArm);
			if (minSf&0x01)
				minArm = GetArm(minSf<<1|minSf);
			else
				minArm = GetArm(minSf|minSf>>1);
			uint8_t maxSf = GetSpreadingFactors(maxArm);
			if (maxSf == 255)
				break;
			if (maxSf&(0x01<<(m_sfs-1)))
				maxArm = GetArm(maxSf>>1|maxSf);
			else
				maxArm = GetArm(maxSf<<1|maxSf);
		}
		minArm = m_lastSelection+1;
		maxArm = m_lastSelection+1;
		for (uint8_t i = 0; i< m_combinations; i++)
		{
			uint8_t minSf = GetSpreadingFactors(minArm);
			minArm = GetArm(minSf>>1&minSf);
			uint8_t maxSf = GetSpreadingFactors(maxArm);
			maxArm = GetArm(maxSf<<1&maxSf);
			if ((minSf>>1&minSf) == 0)
				break;
			for (uint32_t armi = minArm; armi <= maxArm; armi++)
			{
				UpdateValue(m_beta,observation,armi);
			}
		}

		
	}

	void SfMab::UpdateValue (double beta, double observation, uint32_t arm)
	{
		if (m_values[arm-1] == 1)
			m_values[arm-1] = .1+.9*observation;
		else
		{
			m_values[arm-1] *= (1-beta);
			m_values[arm-1] += beta * observation;
		}
	}

	void SfMab::PrintValues ()
	{
		for (uint32_t i = 0; i<m_arms; i++)
			std::cout << m_values[i] << " ";
		std::cout << std::endl;
	}
	
	void SfMab::SetBeta (double beta)
	{
		m_beta=beta;
	}

	double SfMab::GetBeta ()
	{
		return m_beta;
	}
	
	void SfMab::SetEpsilon (uint8_t epsilon)
	{
		m_epsilon=epsilon;
	}

	uint8_t SfMab::GetEpsilon ()
	{
		return m_epsilon;
	}

	uint8_t SfMab::GetSf()
	{
		if (std::experimental::randint(1,100) <= m_epsilon)
			m_lastSelection = std::experimental::randint(0,m_arms-1);
		else
		{
			uint32_t maxIndex = 0;
			for (uint32_t i = 1; i<m_arms; i++)
				if (m_values[i] > m_values[maxIndex])
					maxIndex = i;
			m_lastSelection = maxIndex;
		//std::cout << (uint32_t)m_lastSelection << std::endl;	
		}
		return GetSpreadingFactors (m_lastSelection+1);
	}

} // namespace ns3

