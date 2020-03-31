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
 * Author: Brecht Reynders
 */
#include "ns3/non-communicating-net-device.h"
#include <ns3/okumura-hata-propagation-loss-model.h>
#include <ns3/core-module.h>
#include <ns3/packet.h>
#include <ns3/lora-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/mobility-module.h>
#include <ns3/energy-module.h>
#include <ns3/spectrum-value.h>
#include <ns3/spectrum-analyzer.h>
//#include <ns3/log.h>
//#include <string>
#include <iostream>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/trace-helper.h>
#include <ns3/drop-tail-queue.h>
#include <unordered_map>
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include <ns3/gw-trailer.h>

NS_LOG_COMPONENT_DEFINE ("lora");

using namespace ns3;
using namespace std;

/////////////////////////////////
// Configuration
/////////////////////////////////
//std::vector <int, double> marked_list;
double length = 3000;			//!< Square city with length as distance
int pktsize = 51;              //!< size of packets, in bytes
int duration = 24*60*60;	//!< Duration of the simulation
double interval = 10;        // interval between packets, minutes
bool verbose = false;          // enable logging (different from trace)
bool nakagami = true;         // enable nakagami path loss
bool dynamic = false;          // enable random moving of pan node
uint32_t nSensors = 10; // numbenir of sent packets
uint32_t nGateways = 1; // numbenir of sent packets
Ptr<OutputStreamWrapper> m_stream = 0; // stream for waterfallcurve
Ptr<UniformRandomVariable> randT = CreateObject<UniformRandomVariable> ();
uint32_t interT = 600; // time between a retransmissions.
std::unordered_map<uint32_t,std::tuple<uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t> > errorMap;
//errormap: transmitted, received, received unique, received original, xlocation, ylocation
std::unordered_map<uint32_t,Ptr<LoRaNetDevice> > deviceMap;
Mac32Address server;
NetDeviceContainer gateways;
/////////////////////////////////
// End configuration
/////////////////////////////////

	uint32_t 
GetClosestGateway (Ptr<MobilityModel> location)
{
	uint32_t closestNode; 
	double smallestDistance = 99999999;
	for (uint32_t i = 0; i<gateways.GetN (); i++)
	{
		double distanceToI = location->GetDistanceFrom (gateways.Get(i)->GetNode ()->GetObject<MobilityModel> ());
		if (distanceToI < smallestDistance)
		{
			smallestDistance = distanceToI;
			closestNode = gateways.Get(i)->GetNode ()->GetId();
		}
	}
	return closestNode;
}


/// Save that teh message has been transmitted
	void
Transmitted (const Ptr<const Packet> packet)
{
	Ptr<Packet> copy = packet->Copy();
	LoRaMacHeader header;
	copy->RemoveHeader(header);
	uint32_t addr = Mac32Address::ConvertFrom(header.GetAddr()).GetUInt();
	//std::tuple<uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t> tuple = errorMap[addr];
	std::get<0>(errorMap[addr])++;// = std::make_tuple(++std::get<0>(tuple),std::get<1>(tuple),std::get<2>(tuple),std::get<3>(tuple),std::get<4>(tuple));
}

// save that a message has been received
	void
Received (const Ptr<const Packet> packet)
{
	Ptr<Packet> copy = packet->Copy();
	LoRaMacHeader header;
	copy->RemoveHeader(header);
	GwTrailer trailer;
	copy->RemoveTrailer (trailer);
	uint32_t addr = Mac32Address::ConvertFrom(header.GetAddr()).GetUInt();
	std::get<1>(errorMap[addr])++;
	if ( trailer.GetGateway () == GetClosestGateway (deviceMap[addr]->GetNode ()->GetObject<MobilityModel>())) 
		std::get<3>(errorMap[addr])++;
}

// save that a message has been uniquely received
	void
ReceivedUnique (const Ptr<const Packet> packet)
{
	Ptr<Packet> copy = packet->Copy();
	LoRaMacHeader header;
	copy->RemoveHeader(header);
	uint32_t addr = Mac32Address::ConvertFrom(header.GetAddr()).GetUInt();
	std::get<2>(errorMap[addr])++;
}



/////////////////////////////
// THIS IS THE ACTUAL CODE //
/////////////////////////////
	int
mainBody ()
{
	randT->SetAttribute("Max",DoubleValue(600));
	//Enable checksum (FCS)
	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

	// Logging
	if (verbose)
	{
		LogComponentEnableAll (LOG_PREFIX_TIME);
		LogComponentEnableAll (LOG_PREFIX_FUNC);
	}

	//Create nodes
	NodeContainer loraNetworkNode;
	loraNetworkNode.Create (1);
	NodeContainer loraCoordinatorNodes;
	loraCoordinatorNodes.Create (nGateways);
	NodeContainer loraDeviceNodes;
	loraDeviceNodes.Create(nSensors);
	NodeContainer loraBackendNodes(loraNetworkNode, loraCoordinatorNodes);

	//Create mobility of basestations
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> basePositionList = CreateObject<ListPositionAllocator> ();
	basePositionList->Add (Vector (0.0,0.0,0.0)); //network	
	basePositionList->Add (Vector (1000,1000,50.0)); //main base station	
	mobility.SetPositionAllocator (basePositionList);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install(loraBackendNodes);


	//Mobility nodes
	std::cout << "Create mobility of nodes" << std::endl;
	MobilityHelper mobility2;
	Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator>();
	for(uint32_t nodePositionsAssigned = 0; nodePositionsAssigned < nSensors; nodePositionsAssigned++){
		double x,y;
		do{
		x = randT->GetInteger(0,length);
		y = randT->GetInteger(0,length);
		}
		while ((x-1000)*(x-1000)+(y-1000)*(y-1000) > 1000*1000);
		std::cout << x << "," << y << std::endl;
		nodePositionList->Add (Vector (x,y,1.0));
	}
	mobility2.SetPositionAllocator (nodePositionList);
	mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility2.Install (loraDeviceNodes);
	


	//Channel
	std::cout << "Create channel" << std::endl;
	SpectrumChannelHelper channelHelper;
	channelHelper.SetChannel ("ns3::MultiModelSpectrumChannel");
	if (nakagami)
	{
		channelHelper.AddPropagationLoss ("ns3::OkumuraHataPropagationLossModel","Frequency",DoubleValue(868e6));
		channelHelper.AddPropagationLoss ("ns3::NakagamiPropagationLossModel","m0",DoubleValue(1),"m1",DoubleValue(1),"m2",DoubleValue(1));
	}
	else
	{
		channelHelper.AddPropagationLoss ("ns3::OkumuraHataPropagationLossModel","Frequency",DoubleValue(868e6));
	}
	channelHelper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	Ptr<SpectrumChannel> channel = channelHelper.Create ();
	LoRaHelper lorahelper;
	lorahelper.SetChannel (channel);


	// Configure gateways
	std::cout << "Create gateways" << std::endl;
	gateways = lorahelper.InstallGateways (loraCoordinatorNodes);

	// Create the nodes
	std::cout << "Create nodes" << std::endl;
	NetDeviceContainer loraNetDevices;
	loraNetDevices = lorahelper.Install (loraDeviceNodes);
	
	// create energy source
	LoRaEnergySourceHelper sourceHelper;
	EnergySourceContainer energySources = sourceHelper.Install(loraDeviceNodes);
	LoRaRadioEnergyModelHelper radioHelper;
	DeviceEnergyModelContainer deviceModels = radioHelper.Install (loraNetDevices, energySources);

	// Connect gateways with network
	std::cout << "Create wired network" << std::endl;
	CsmaHelper csma;
	csma.SetChannelAttribute ("DataRate", StringValue ("1000Mbps"));
	csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (60)));

	std::cout << "Connect devices with wired network" << std::endl;
	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install (loraBackendNodes);

	std::cout << "Install IP/TCP" << std::endl;
	InternetStackHelper stack;
	stack.Install (loraBackendNodes);
	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = address.Assign (csmaDevices);

	// set addresses 
	std::cout << "Set the addresses" << std::endl;
	lorahelper.FinishGateways (loraCoordinatorNodes, gateways, interfaces.GetAddress(0));
	// Reset the power after each succesfull message
	lorahelper.InstallNetworkApplication("ns3::LoRaNoPowerApplication");
	Ptr<LoRaNetwork> loraNetwork = lorahelper.InstallBackend (loraNetworkNode.Get (0),loraNetDevices);

	// Let the nodes generate data
	std::cout << " Generate data from the nodes in the LoRa network" << std::endl;
	ApplicationContainer apps = lorahelper.GenerateTraffic (randT, loraDeviceNodes, pktsize, 0, duration, 120, false);

	// hookup functions to the netdevices of each node to measure the performance
	for (uint32_t i = 0; i< loraNetDevices.GetN(); i++)
	{
		deviceMap[ Mac32Address::ConvertFrom(loraNetDevices.Get(i)->GetAddress()).GetUInt()]=DynamicCast<LoRaNetDevice>(loraNetDevices.Get(i));
		uint32_t x  = loraNetDevices.Get(i)->GetNode()->GetObject<MobilityModel>()->GetPosition ().x;
		uint32_t y  = loraNetDevices.Get(i)->GetNode()->GetObject<MobilityModel>()->GetPosition ().y;
		errorMap[ Mac32Address::ConvertFrom(loraNetDevices.Get(i)->GetAddress()).GetUInt()] = make_tuple (0,0,0,0,x,y);
		DynamicCast<LoRaNetDevice>(loraNetDevices.Get(i))->TraceConnectWithoutContext ("MacTx",MakeCallback(&Transmitted));
	}

	// hookup functions to the network for measuring performance
	loraNetwork->TraceConnectWithoutContext("NetRx",MakeCallback(&ReceivedUnique));
	loraNetwork->TraceConnectWithoutContext("NetPromiscRx",MakeCallback(&Received));	

	// Start the simulation
	std::cout << "start the fun" << std::endl;
	Simulator::Stop (Seconds (duration));
	Simulator::Run ();

	for (uint8_t i = 1; i<= nSensors; i++)
	{
	//	std::cout << energySources.GetSize() << std::endl;
		std::cout << energySources.Get(i-1)->GetRemainingEnergy();
	}

	return 0;
}

	int 
main (int argc, char** argv)
{
	AsciiTraceHelper ascii;
	m_stream = ascii.CreateFileStream("data.csv");
	*m_stream->GetStream() << "#Scenario " << (int)nSensors <<  " nodes with " << (int)nGateways << " gateways and " << (int)nSensors << " nodes on a square field with side " << length << " meter" <<std::endl;
	for (uint8_t iterationI=0;iterationI<10;iterationI++){
		std::cout << "Iteration: " << (int)iterationI << std::endl;
		mainBody();
		for ( auto it = errorMap.begin(); it !=errorMap.end();++it)
		{
			uint32_t addr = it->first;
			Ptr<LoRaNetDevice> netdevice = deviceMap[addr];
			std::tuple<uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t> tuple = it->second;
			// print ID, transmitted, received, received unique, received at closest gateway, x coords, y coords, get average amount of retransmissions, get average time of transmissions, number of missed messages, amount of received messages.
			*m_stream->GetStream() << addr << "," << std::get<0>(tuple)<< "," << std::get<1>(tuple) << "," <<   std::get<2>(tuple) << "," << std::get<3>(tuple) << "," << std::get<4>(tuple) << "," << std::get<5>(tuple) <<","<< netdevice->GetAvgRetransmissionCount() << "," << netdevice->GetAvgTime() << "," << netdevice->GetMissed() << "," << netdevice->GetArrived() << std::endl;
		}
		errorMap.clear();
		Simulator::Destroy ();
	}
	return 0;
}

