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

#ifndef SF_MAB_H 
#define SF_MAB_H

#include <stdint.h>

namespace ns3 {

/**
*
*
*/
class SfMab
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  SfMab (uint32_t combinations,uint8_t spreadingfactors);
  SfMab ();
  virtual ~SfMab();

	

	/**
		*\brief Newpacket rececives new packet and parses it
		*/
	void NewObservation (double observation);
	void SetBeta (double beta);
	double GetBeta ();
	void SetEpsilon (uint8_t epsilon);
	uint8_t GetEpsilon ();

	uint8_t GetSf();
	
	// LinkAdrAns
	/**
		* ConfirmPower saves the power setting when the device acknowledges this.
		*	
		*	\param	address	The address that acknowledged.
		*/
	//virtual void ConfirmPower (const Address& address);
	uint8_t GetSpreadingFactors (uint32_t arm); 
	uint32_t GetArm (uint8_t spreadingfactors);
	void PrintValues ();

private:
	uint32_t m_combinations;
	uint8_t m_lastSelection;
	uint8_t m_arms;
	uint8_t m_sfs;

	double* m_values = {};
	uint8_t m_epsilon;
	double m_beta;

	void UpdateValue (double beta, double observation, uint32_t arm);

};

} // namespace ns3

#endif /* SF_MAB_H*/
