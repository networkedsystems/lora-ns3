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
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 *         Peishuo Li <pressthunder@gmail.com>
 *				 Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#include "ns3/energy-source.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/lora-radio-energy-model.h"
#include "ns3/lora-phy.h"

NS_LOG_COMPONENT_DEFINE ("LoRaRadioEnergyModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoRaRadioEnergyModel);

TypeId
LoRaRadioEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaRadioEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .AddConstructor<LoRaRadioEnergyModel> ()
    .AddAttribute ("IdleCurrentA",
                   "The default radio Idle current in Ampere.",
                   DoubleValue (0.00000052),
                   MakeDoubleAccessor (&LoRaRadioEnergyModel::SetIdleCurrentA,
                                       &LoRaRadioEnergyModel::GetIdleCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxCurrentA",
                   "The radio Tx current.",
                   DoubleValue (0.007),
                   MakeDoubleAccessor (&LoRaRadioEnergyModel::SetTxCurrentA,
                                       &LoRaRadioEnergyModel::GetTxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxCurrentA",
                   "The radio Rx current.",
                   DoubleValue (0.0005),
                   MakeDoubleAccessor (&LoRaRadioEnergyModel::SetRxCurrentA,
                                       &LoRaRadioEnergyModel::GetRxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&LoRaRadioEnergyModel::m_totalEnergyConsumption),
                     "ns3::TracedValue::DoubleCallback")
    .AddTraceSource ("CurrentEnergyState",
                     "Current Phy layer state and corresponding energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&LoRaRadioEnergyModel::m_EnergyStateLogger),
                     "ns3::LoRaRadioEnergyModel::EnergyStateTracedCallback")
  ; 
  return tid;
}

LoRaRadioEnergyModel::LoRaRadioEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  //m_energyDepletionCallback.Nullify ();
  m_source = NULL;
  m_currentState = LoRaPhy::State::IDLE;
  m_sourceEnergyUnlimited = 0;
  m_remainingBatteryEnergy = 0;
  m_sourcedepleted = 0;
}

LoRaRadioEnergyModel::~LoRaRadioEnergyModel ()
{
  NS_LOG_FUNCTION (this);
}

void
LoRaRadioEnergyModel::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
  m_energyToDecrease = 0;
  m_remainingBatteryEnergy = m_source->GetInitialEnergy();
}

double
LoRaRadioEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

double
LoRaRadioEnergyModel::GetIdleCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_IdleCurrentA;
}

void
LoRaRadioEnergyModel::SetIdleCurrentA (double idleCurrentA)
{
  NS_LOG_FUNCTION (this << idleCurrentA);
  m_IdleCurrentA = idleCurrentA;
}

double
LoRaRadioEnergyModel::GetTxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_TxCurrentA;
}

void
LoRaRadioEnergyModel::SetTxCurrentA (double txCurrentA)
{
  NS_LOG_FUNCTION (this << txCurrentA);
  m_TxCurrentA = txCurrentA;
}

double
LoRaRadioEnergyModel::GetRxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_RxCurrentA;
}

void
LoRaRadioEnergyModel::SetRxCurrentA (double rxCurrentA)
{
  NS_LOG_FUNCTION (this << rxCurrentA);
  m_RxCurrentA = rxCurrentA;
}

LoRaPhy::State
LoRaRadioEnergyModel::GetCurrentState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void
LoRaRadioEnergyModel::HandleEnergyDepletion (){
}

void 
LoRaRadioEnergyModel::HandleEnergyRecharged (){
}

void
LoRaRadioEnergyModel::ChangeLoRaState (LoRaPhy::State newstate)
{
  NS_LOG_FUNCTION (this << newstate);

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0); // check if duration is valid

  // energy to decrease = current * voltage * time
      m_energyToDecrease = 0.0;
      double supplyVoltage = m_source->GetSupplyVoltage ();

      switch (m_currentState)
        {
					case LoRaPhy::State::IDLE:
          m_energyToDecrease = duration.GetSeconds () * m_IdleCurrentA * supplyVoltage;
          break;
        case LoRaPhy::State::TX:
          m_energyToDecrease = duration.GetSeconds () * m_TxCurrentA * supplyVoltage;
          break;
        case LoRaPhy::State::RX:
          m_energyToDecrease = duration.GetSeconds () * m_RxCurrentA * supplyVoltage;
          break;
				default:
          NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);
        }

      // update total energy consumption
      m_totalEnergyConsumption += m_energyToDecrease;

      // update last update time stamp
      m_lastUpdateTime = Simulator::Now ();

      // notify energy source
      m_source->UpdateEnergySource ();

  if (!m_sourcedepleted)
    {
      SetLoRaRadioState (newstate);
      NS_LOG_DEBUG ("LoRaRadioEnergyModel:Total energy consumption is " <<
                    m_totalEnergyConsumption << "J");
    }
}

 void
 LoRaRadioEnergyModel::ChangeState (int newState)
{
}

/*
 * Private functions start here.
 */

void
LoRaRadioEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = NULL;
}


double
LoRaRadioEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  switch (m_currentState)
    {
    case LoRaPhy::State::IDLE:
      return m_IdleCurrentA;
    case LoRaPhy::State::TX:
      return m_TxCurrentA;
    case LoRaPhy::State::RX:
      return m_RxCurrentA;
    default:
      NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state:" << m_currentState);
    }
}


void
LoRaRadioEnergyModel::SetLoRaRadioState (const LoRaPhy::State state)
{
  NS_LOG_FUNCTION (this << state);

  std::string preStateName;
  switch (m_currentState)
    {
		case LoRaPhy::State::IDLE:
      preStateName = "IDLE";
      break;
    case LoRaPhy::State::TX:
      preStateName = "TX";
      break;
		case LoRaPhy::State::RX: 
      preStateName = "RX";
      break;
  default:
    NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);
  }

  m_currentState = state;
  std::string curStateName;
  switch (state)
    {
    case LoRaPhy::State::IDLE:
      curStateName = "IDLE";
      break;
    case LoRaPhy::State::RX:
      curStateName = "RX";
      break;
    case LoRaPhy::State::TX:
      curStateName = "TX";
      break;
  default:
    NS_FATAL_ERROR ("LoRaRadioEnergyModel:Undefined radio state: " << m_currentState);
  }

  m_remainingBatteryEnergy = m_source -> GetRemainingEnergy();

  m_EnergyStateLogger (preStateName, curStateName, m_sourceEnergyUnlimited, m_energyToDecrease, m_remainingBatteryEnergy, m_totalEnergyConsumption);

  NS_LOG_DEBUG ("LoRaRadioEnergyModel:Switching to state: " << curStateName <<
                " at time = " << Simulator::Now ());
}


// -------------------------------------------------------------------------- //


/*
 * Private function state here.
 */



} // namespace ns3
