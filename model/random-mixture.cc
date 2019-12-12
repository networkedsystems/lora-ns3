#include "random-mixture.h"
#include <ns3/object.h>
#include <ns3/random-variable-stream.h>
#include <ns3/pointer.h>
#include <ns3/log.h>
#include <array>
#include <ns3/double.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("RandomMixture");

NS_OBJECT_ENSURE_REGISTERED (RandomMixture);


  /**
   * \brief Register this type.
   * \return The object TypeId.
   */
  TypeId RandomMixture::GetTypeId (void)
	{
  	static TypeId tid = TypeId ("ns3::RandomMixture")
    	.SetParent<RandomVariableStream> ()
    	.AddConstructor<RandomMixture> ()
  	;
  	return tid;
	}

  /**
   * \brief Creates a uniform distribution RNG with the default range.
   */
	RandomMixture::RandomMixture ()
	{
		m_randsTotal = 0;
		m_weightsTotal = 0;
		m_weightRand = CreateObject<UniformRandomVariable> ();
		m_weightRand->SetAttribute("Min",DoubleValue(0));
	}

	RandomMixture::~RandomMixture ()
	{
		for (uint8_t i = 0; i< 10; i++)
		{
			m_rands[i] = 0;
			m_weights[i] = 0;
		}
	}

  /**
   * \brief Get the next random value as a double drawn from the distribution.
   * \return A floating point random value.
   * \note The upper limit is excluded from the output range.
  */
  double RandomMixture::GetValue (void)
	{
		return GetRandomStream()->GetValue();
	}

	Ptr<RandomVariableStream> RandomMixture::GetRandomStream (void)
		{
		double runningWeight = 0;
		double distributionWeight = m_weightRand->GetValue();
		uint8_t i = 0;
		for (i = 0; i< m_randsTotal; i++)
		{
			if ( runningWeight + m_weights[i] < distributionWeight)
				runningWeight+= m_weights[i];
			else
				break;
		}
		return m_rands[i];
	}

  /**
   * \brief Get the next random value as an integer drawn from the distribution.
   * \return  An integer random value.
   * \note The upper limit is included in the output range.
   */
  uint32_t RandomMixture::GetInteger (void)
	{
		return GetRandomStream ()->GetInteger ();
	}

	/**
  	*
  	*
  	*/
	void RandomMixture::AddNewDistribution (double weight, Ptr<RandomVariableStream> random)
	{
		NS_ASSERT(m_randsTotal < 10);
		m_weights[m_randsTotal] = weight;
		m_rands[m_randsTotal] = random;
		m_randsTotal++;
		m_weightsTotal += weight;
		m_weightRand->SetAttribute("Max",DoubleValue(m_weightsTotal));
	}
}  
