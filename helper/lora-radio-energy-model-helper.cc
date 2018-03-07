/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
 * Authors: 
 *          Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */

#include "lora-radio-energy-model-helper.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-phy.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/log.h"
#include <ns3/type-id.h>

namespace ns3 {

LoRaRadioEnergyModelHelper::LoRaRadioEnergyModelHelper ()
{
  m_radioEnergy.SetTypeId ("ns3::LoRaRadioEnergyModel");
//  m_depletionCallback.Nullify ();
}

LoRaRadioEnergyModelHelper::~LoRaRadioEnergyModelHelper ()
{
}

void
LoRaRadioEnergyModelHelper::Set (std::string name, const AttributeValue &v)
{
  m_radioEnergy.Set (name, v);
}

//void
//LoRaRadioEnergyModelHelper::SetDepletionCallback (
//  LoRaRadioEnergyModel::LoRaRadioEnergyDepletionCallback callback)
//{
//  m_depletionCallback = callback;
//}

/*
 * Private function starts here.
 */

Ptr<DeviceEnergyModel>
LoRaRadioEnergyModelHelper::DoInstall (Ptr<NetDevice> device,
                                       Ptr<EnergySource> source) const
{
  NS_ASSERT (device != NULL);
  NS_ASSERT (source != NULL);

 std::string deviceName = device->GetInstanceTypeId ().GetName ();
 if (deviceName.compare ("ns3::LoRaNetDevice") != 0 && 0 != device->GetInstanceTypeId().GetParent ().GetName ().compare ("ns3::LoRaNetDevice"))
   {
	 NS_FATAL_ERROR ("NetDevice type is not LoRaNetDevice!");
   }

  Ptr<LoRaRadioEnergyModel> model = m_radioEnergy.Create ()->GetObject<LoRaRadioEnergyModel> ();
  NS_ASSERT (model != NULL);

  model->SetEnergySource (source);
  Ptr<LoRaNetDevice> LoRaDevice = DynamicCast<LoRaNetDevice>(device);
  source->AppendDeviceEnergyModel (model);

  //track transceiver state
  Ptr<LoRaPhy> LoRaPhy = LoRaDevice->GetPhy ();
  LoRaPhy -> TraceConnectWithoutContext ("StateValue",MakeCallback(&LoRaRadioEnergyModel::ChangeLoRaState, model));

  return model;
}

} // namespace ns3
