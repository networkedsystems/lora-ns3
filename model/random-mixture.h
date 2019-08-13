/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
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

#ifndef RANDOM_MIXTURE_H
#define RANDOM_MIXTURE_H

#include <ns3/random-variable-stream.h>
#include <array>

namespace ns3
{


class RandomMixture : public RandomVariableStream
{
public:
  /**
   * \brief Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Creates a uniform distribution RNG with the default range.
   */
  RandomMixture ();
  ~RandomMixture ();

  // Inherited from RandomVariableStream
  /**
   * \brief Get the next random value as a double drawn from the distribution.
   * \return A floating point random value.
   * \note The upper limit is excluded from the output range.
  */
  virtual double GetValue (void);
  /**
   * \brief Get the next random value as an integer drawn from the distribution.
   * \return  An integer random value.
   * \note The upper limit is included in the output range.
   */
  virtual uint32_t GetInteger (void);

	/**
  	*
  	*
  	*/
	void AddNewDistribution (double weight, Ptr<RandomVariableStream> random);
  
private:
	Ptr<RandomVariableStream> m_rands[10];
	double m_weights[10];
	Ptr<RandomVariableStream> m_weightRand;
	double m_weightsTotal;
	uint32_t m_randsTotal;
	
	Ptr<RandomVariableStream> GetRandomStream ();

};  // class RandomMixture 
}

#endif
