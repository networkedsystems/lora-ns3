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
 * Author: kwong yin <kwong-sang.yin@boeing.com>
 */

#include "lora-mac-command.h"
#include "commands/link-check-ans.h"
#include "commands/link-check-req.h"
#include "commands/link-adr-req.h"
#include "commands/link-adr-ans.h"
#include "commands/duty-cycle-req.h"
#include "commands/duty-cycle-ans.h"
#include "commands/rx-param-setup-req.h"
#include "commands/rx-param-setup-ans.h"
#include "commands/dev-status-req.h"
#include "commands/dev-status-ans.h"
#include "commands/new-channel-req.h"
#include "commands/new-channel-ans.h"
#include "commands/rx-timing-setup-req.h"
#include "commands/rx-timing-setup-ans.h"
#include "ns3/log.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LoRaMacCommand);
NS_LOG_COMPONENT_DEFINE ("LoRaMacCommand");



LoRaMacCommand::~LoRaMacCommand()
{
}

LoRaMacCommandCid
LoRaMacCommand::GetType (void) const
{
	return m_cid;
}

LoRaMacCommandCid
LoRaMacCommand::GetType (uint8_t cid) 
{
	NS_LOG_FUNCTION (cid);
  switch (cid)
    {
    case 2:
      return LINK_CHECK;
      break;
    case 3:
      return LINK_ADR;
      break;
    case 4:
      return DUTY_CYCLE;
      break;
    case 5:
      return RX_PARAM_SETUP;
      break;
    case 6:
      return DEV_STATUS;
      break;
    case 7:
      return NEW_CHANNEL;
      break;
    case 8:
      return RX_TIMING;
      break;
    default:
      return LORA_COMMAND_PROPRIETARY;
    }
}

void
LoRaMacCommand::SetType (LoRaMacCommandCid cid)
{
	NS_LOG_FUNCTION (this);
  m_cid = cid;
}

void 
LoRaMacCommand::SetDirection (LoRaMacCommandDirection direction)
{
	NS_LOG_FUNCTION (this);
	m_direction = direction;
}

TypeId
LoRaMacCommand::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoRaMacCommand")
								.SetParent<Object>();
  return tid;
}

TypeId
LoRaMacCommand::GetInstanceTypeId (void) const
{
	NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
LoRaMacCommand::Print (std::ostream &os) const
{
	NS_LOG_FUNCTION (this);
  os << "Command = " << (uint32_t)m_cid; 
}

Ptr<LoRaMacCommand>
LoRaMacCommand::CommandBasedOnCid (uint8_t cid, LoRaMacCommandDirection direction)
{
	NS_LOG_FUNCTION (cid << direction);
	if (direction == TOBASE)
	{
		switch (static_cast<LoRaMacCommandCid> ( cid ))
		{
			case LINK_CHECK:
				return CreateObject<LinkCheckReq> ();
				break;
			case LINK_ADR:
				return CreateObject<LinkAdrAns> ();
				break;
			case DUTY_CYCLE:
				return CreateObject<DutyCycleAns> ();
				break;
			case RX_PARAM_SETUP:
				return CreateObject<RxParamSetupAns> ();
				break;
			case DEV_STATUS:
				return CreateObject<DevStatusAns> ();
				break;
			case NEW_CHANNEL:
				return CreateObject<NewChannelAns> ();
				break;
			case RX_TIMING:
				return CreateObject<RxTimingSetupAns> ();
				break;
			default:
				NS_FATAL_ERROR("I don't know what to do with it..");
		}
	}
	else
	{
		switch (static_cast<LoRaMacCommandCid> ( cid ))
		{
			case LINK_CHECK:
				return CreateObject<LinkCheckAns> ();
				break;
			case LINK_ADR:
				return CreateObject<LinkAdrReq> ();
				break;
			case DUTY_CYCLE:
				return CreateObject<DutyCycleReq> ();
				break;
			case RX_PARAM_SETUP:
				return CreateObject<RxParamSetupReq> ();
				break;
			case DEV_STATUS:
				return CreateObject<DevStatusReq> ();
				break;
			case NEW_CHANNEL:
				return CreateObject<NewChannelReq> ();
				break;
			case RX_TIMING:
				return CreateObject<RxTimingSetupReq> ();
				break;
				//add here also the other ones
			default:
				NS_FATAL_ERROR("No such cid");
		}
	}
	return CreateObject<LinkAdrReq> ();
}

} //namespace ns3

