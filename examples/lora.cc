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
#include <ns3/rectangle.h>
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


class Hasher_brecht {
public:
    size_t operator() (const Address& key) const {     // the parameter type should be the same as the type of key of unordered_map
        uint64_t hash = 0;
				uint8_t buffer[8];
				memset(buffer,0,8);
				key.CopyAllTo(buffer,key.GetLength()+2);
        for(uint32_t i = 0; i < 8; i++) {
            hash += ((uint64_t) buffer[i]) << i;
        }
        return hash;
    }
};


/////////////////////////////////
// Configuration
/////////////////////////////////
//std::vector <int, double> marked_list;
bool ack = false;
bool rslora = false;
bool learning = false;
bool optimized = false;
bool monitorEnergy = false;
bool interference = false;
bool randomSend = false;
double length = 1000;			//!< Square city with length as distance
double iterationCount = 5;			//!< Square city with length as distance
int pktsize = 51;              //!< size of packets, in bytes
int duration = 24*60*60;	//!< Duration of the simulation
int measurementStart = 0;	//!< Start of the measured simulation (gives time to settle) 
double interval = 120;        // interval between packets, minutes
bool verbose = false;          // enable logging (different from trace)
bool nakagami = true;         // enable nakagami path loss
bool dynamic = false;          // enable random moving of pan node
uint32_t nSensors = 500; // numbenir of sent packets
uint32_t nGateways = 1; // numbenir of sent packets
uint32_t reportingInterval = 0; // numbenir of sent packets
Ptr<OutputStreamWrapper> m_stream = 0;
std::stringstream filename;
Ptr<UniformRandomVariable> randT = CreateObject<UniformRandomVariable> ();
std::unordered_map<Address, Ptr<NetDevice>, Hasher_brecht> deviceMap;
//errormap: transmitted, received, received unique, received original, xlocation, ylocation
std::unordered_map<Address, std::tuple<uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t>, Hasher_brecht> errorMap;
Mac32Address server;
NetDeviceContainer gateways;
uint8_t offsets [7] = {2,3,3,1,2,2,3};
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
	if (Simulator::Now().GetSeconds() > measurementStart)
	{
		Ptr<Packet> copy = packet->Copy();
		LoRaMacHeader header;
		copy->RemoveHeader(header);
		Address addr = (header.GetAddr());
		//std::tuple<uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t> tuple = errorMap[addr];
		std::get<0>(errorMap[addr])++;// = std::make_tuple(++std::get<0>(tuple),std::get<1>(tuple),std::get<2>(tuple),std::get<3>(tuple),std::get<4>(tuple));
	}
}

// save that a message has been received
	void
Received (const Ptr<const Packet> packet)
{
	if (Simulator::Now().GetSeconds() > measurementStart)
	{
	Ptr<Packet> copy = packet->Copy();
	LoRaMacHeader header;
	copy->RemoveHeader(header);
	GwTrailer trailer;
	copy->RemoveTrailer (trailer);
	Address addr = (header.GetAddr());
	std::get<1>(errorMap[addr])++;
	if ( trailer.GetGateway () == GetClosestGateway (deviceMap[addr]->GetNode ()->GetObject<MobilityModel>())) 
		std::get<3>(errorMap[addr])++;
	}
}

// save that a message has been uniquely received
	void
ReceivedUnique (const Ptr<const Packet> packet)
{
	if (Simulator::Now().GetSeconds() > measurementStart)
	{
		Ptr<Packet> copy = packet->Copy();
		LoRaMacHeader header;
		copy->RemoveHeader(header);
		Address addr = (header.GetAddr());
		std::get<2>(errorMap[addr])++;
	}
}

void StoreData(uint32_t timeInSimulation)
{
		for ( auto it = errorMap.begin(); it !=errorMap.end();++it)
		{
			Address addr = it->first;
			Ptr<LoRaNetDevice> netdevice = DynamicCast<LoRaNetDevice>(deviceMap[addr]);
			std::tuple<uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t> tuple = it->second;
			if ( netdevice!=0)
			{
				// print ID, transmitted, received, received unique, received at closest gateway, x coords, y coords, get average amount of retransmissions, get average time of transmissions, number of missed messages, amount of received messages.
				*m_stream->GetStream() << addr << "," << std::get<0>(tuple)<< "," << std::get<1>(tuple) << "," <<   std::get<2>(tuple) << "," << std::get<3>(tuple) << "," << std::get<4>(tuple) << "," << std::get<5>(tuple) <<","<< netdevice->GetAvgRetransmissionCount() << "," << netdevice->GetAvgTime() << "," << netdevice->GetMissed() << "," << netdevice->GetArrived() << "," << timeInSimulation;
				if (netdevice->GetNode()->GetObject<EnergySourceContainer>())
				{
					*m_stream->GetStream() << "," << netdevice->GetNode()->GetObject<EnergySourceContainer>()->Get(0)->GetRemainingEnergy() << ",1"<< std::endl;
				}
				else
					*m_stream->GetStream() << ",0,1"<< std::endl;
			}
			else
			{
				*m_stream->GetStream() << addr << "," << std::get<0>(tuple)<< "," << std::get<1>(tuple) << "," <<   std::get<2>(tuple) << "," << std::get<3>(tuple) << "," << std::get<4>(tuple) << "," << std::get<5>(tuple) <<",0,0,0,0,0,0" << std::endl;
			}
		}
}

void
ScheduleStoring()
{
	StoreData(Simulator::Now().GetSeconds());
	Simulator::Schedule(Seconds(reportingInterval),&ScheduleStoring);
}



/////////////////////////////
// THIS IS THE ACTUAL CODE //
/////////////////////////////
	int
mainBody ()
{
	randT->SetAttribute("Max",DoubleValue(interval));
	//Enable checksum (FCS)
	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

	// Logging
	if (verbose)
	{
		LogComponentEnableAll (LOG_PREFIX_TIME);
		LogComponentEnableAll (LOG_PREFIX_FUNC);
	}

	//Set the information Callback
	if (reportingInterval > 0)
		Simulator::Schedule(Seconds(reportingInterval),&ScheduleStoring);

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
	// place gateways in a hexagonal structure
	basePositionList->Add (Vector (0.0,0.0,0.0)); //network	
	// center gateway
	basePositionList->Add (Vector (length,length,50));
	// 3 gateway equally spaced around the central gateway
	basePositionList->Add (Vector (length/2,1866*length/1000,50.0)); //main base station	
	basePositionList->Add (Vector (length/2,134*length/1000,50.0)); //main base station	
	basePositionList->Add (Vector (2*length,length,50.0)); //main base station	
	// 3 gateways equally spaced between all other gw's
	basePositionList->Add (Vector (0,length,50.0)); //main base station	
	basePositionList->Add (Vector (1.500*length,1866*length/1000,50.0)); //main base station	
	basePositionList->Add (Vector (1.500*length,134*length/1000,50.0)); //main base station	
	//basePositionList->Add (Vector (length,length,50.0)); //main base station	
	//basePositionList->Add (Vector (length/3,length,50.0)); //main base station	
	//basePositionList->Add (Vector (4*length/3,1.5774*length,50.0)); //main base station	
	//basePositionList->Add (Vector (length*4/3,.42265*length,50.0)); //main base station
	//basePositionList->Add (Vector (length*2/3,1.5774*length,50.0)); //main base station	
	//basePositionList->Add (Vector (length*2/3,.42265*length,50.0)); //main base station	
	//basePositionList->Add (Vector (5*length/3,length,50.0)); //main base station	
	mobility.SetPositionAllocator (basePositionList);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install(loraBackendNodes);


	//Mobility nodes
	// this is random
	std::cout << "Create mobility of nodes" << std::endl;
	MobilityHelper mobility2;
	Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator>();
	double lengthMax = length;
	if (nGateways>3)
		lengthMax = 1500;
	for(uint32_t nodePositionsAssigned = 0; nodePositionsAssigned < nSensors; nodePositionsAssigned++){
		double x,y;
		do{
			x = randT->GetInteger(0,2*lengthMax);
			y = randT->GetInteger(0,2*lengthMax);
		}
		while ((x-length)*(x-length)+(y-length)*(y-length) > lengthMax*lengthMax);
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
	if (rslora)
	{
		gateways = lorahelper.InstallRsGateways (loraCoordinatorNodes);
		for (uint32_t i = 0; i< gateways.GetN(); i++)
			gateways.Get(i)->SetAttribute ("Offset",UintegerValue(offsets[i]-1));
	}
	else
		gateways = lorahelper.InstallGateways (loraCoordinatorNodes);

	// Create the nodes
	std::cout << "Create nodes" << std::endl;
	NetDeviceContainer loraNetDevices;
	if (rslora)
	{
		loraNetDevices = lorahelper.InstallRs (loraDeviceNodes);
		for (uint32_t i = 0; i< loraNetDevices.GetN (); i++)
		{
			// this is a dirty workaround. Make sure to create first the network and then the gateways
			loraNetDevices.Get(i)->SetAttribute ("Offset2",UintegerValue(offsets[GetClosestGateway(loraNetDevices.Get(i)->GetNode()->GetObject<MobilityModel>())-1]-1));
		}
	}
	else
		loraNetDevices = lorahelper.Install (loraDeviceNodes);


	// Check if reliable
	if (ack)
		for (uint32_t i = 0; i<loraNetDevices.GetN(); i++)
		{
			loraNetDevices.Get(i)->SetAttribute("Reliable",BooleanValue(true));
			
			Ptr<LoRaPhy> temp = StaticCast<LoRaPhy>(StaticCast<LoRaNetDevice>(loraNetDevices.Get(i))->GetPhy());
		}

	// create energy source
	if(monitorEnergy)
	{
		std::cout << "Monitoring energy enabled" << std::endl;
		LoRaEnergySourceHelper sourceHelper;
		sourceHelper.Set("BasicEnergySourceInitialEnergyJ",DoubleValue(100));
		EnergySourceContainer energySources = sourceHelper.Install(loraDeviceNodes);
		LoRaRadioEnergyModelHelper radioHelper;
		DeviceEnergyModelContainer deviceModels = radioHelper.Install (loraNetDevices, energySources);
	}

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
	if (learning)
	{
		lorahelper.InstallNetworkApplication("ns3::LoRaSfControllerApplication");
		//Simulator::Schedule(Seconds(duration-1),&LoRaSfControllerApplication::PrintValues(),DynamicCast<LoRaSfControllerApplication>(apps.get(0)));
	}
	else
	{
		if (optimized)
		{
			lorahelper.InstallNetworkApplication("ns3::LoRaPowerApplication");
		}
		else
		{
			lorahelper.InstallNetworkApplication("ns3::LoRaNoPowerApplication");
		}
	}
	Ptr<LoRaNetwork> loraNetwork = lorahelper.InstallBackend (loraNetworkNode.Get (0),loraNetDevices);

	// Let the nodes generate data
	std::cout << " Generate data from the nodes in the LoRa network" << std::endl;
	ApplicationContainer apps = lorahelper.GenerateTraffic (randT, loraDeviceNodes, pktsize, 0, duration, interval,randomSend);

	// hookup functions to the netdevices of each node to measure the performance
	for (uint32_t i = 0; i< loraNetDevices.GetN(); i++)
	{
		deviceMap[ (loraNetDevices.Get(i)->GetAddress())]=loraNetDevices.Get(i);
		uint32_t x  = loraNetDevices.Get(i)->GetNode()->GetObject<MobilityModel>()->GetPosition ().x;
		uint32_t y  = loraNetDevices.Get(i)->GetNode()->GetObject<MobilityModel>()->GetPosition ().y;
		errorMap[ (loraNetDevices.Get(i)->GetAddress())] = make_tuple (0,0,0,0,x,y);
		DynamicCast<LoRaNetDevice>(loraNetDevices.Get(i))->TraceConnectWithoutContext ("MacTx",MakeCallback(&Transmitted));
	}

	// hookup functions to the network for measuring performance
	loraNetwork->TraceConnectWithoutContext("NetRx",MakeCallback(&ReceivedUnique));
	loraNetwork->TraceConnectWithoutContext("NetPromiscRx",MakeCallback(&Received));	

	// Configure interference
	if (interference)
	{
		std::cout << "Interference Enabled" << std::endl;
		MobilityHelper mobilityInterference;
		Ptr<RandomDiscPositionAllocator> allocator = CreateObject<RandomDiscPositionAllocator>();
		Ptr<RandomVariableStream> radius = CreateObject<UniformRandomVariable>();
		radius->SetAttribute("Max",DoubleValue(2000));
		allocator->SetAttribute("Rho",PointerValue(radius));
		allocator->SetAttribute("X",DoubleValue(1500));
		allocator->SetAttribute("Y",DoubleValue(1500));
		Ptr<RandomVariableStream> speed = CreateObject<ConstantRandomVariable>();
		speed->SetAttribute("Constant",DoubleValue(3e9));
		Ptr<RandomVariableStream> pause = CreateObject<ConstantRandomVariable>();
		pause->SetAttribute("Constant",DoubleValue(0.5));

		mobilityInterference.SetMobilityModel ("ns3::RandomDirection2dMobilityModel","Bounds",RectangleValue(Rectangle(-length*3,length*3,-length*3,length*3)),"Speed",PointerValue(speed),"Pause",PointerValue(pause));
		mobilityInterference.SetPositionAllocator((allocator));
		lorahelper.AddInterference(mobilityInterference);
	}


	// Start the simulation
	std::cout << "start the fun" << std::endl;
	Simulator::Stop (Seconds (duration));
	Simulator::Run ();

	return 0;
}

	int 
main (int argc, char** argv)
{
	CommandLine cmd;
	cmd.AddValue ("ack", "Only send confirmed messages", ack);
	cmd.AddValue ("pktsize", "The size of a packet", pktsize);
	cmd.AddValue ("rate", "Time between 2 consecutive messages", interval);
	cmd.AddValue ("rslora", "Improved LoRa MAC layer", rslora);
	cmd.AddValue ("learning", "CCMAB for learning ideal spreading factor set", learning);
	cmd.AddValue ("gateways", "The amount of gateways (up to 7) (1,4,7 for optimal performance)", nGateways);
	cmd.AddValue ("sensors", "The amount of sensors", nSensors);
	cmd.AddValue ("interference", "Use measured interference", interference);
	cmd.AddValue ("optimized", "Use the best static spreading factor set [haven't used this in a very long time. Use at your own risk, I hard coded a few things]", optimized);
	cmd.AddValue ("length", "Radius of a cell", length);
	cmd.AddValue ("duration", "Duration of a simulation", duration);
	cmd.AddValue ("start", "Starting time of measuring packets", measurementStart);
	cmd.AddValue ("monitorEnergy", "Monitors the energy of the nodes", monitorEnergy);
	cmd.AddValue ("iterationCount", "The amount of repeated simulations", iterationCount);
	cmd.AddValue ("randomSend", "Add randomness to interval", randomSend);
	cmd.AddValue ("reportingInterval","The interval for reporting statistics",reportingInterval);

	cmd.Parse (argc,argv);
	AsciiTraceHelper ascii;
	filename <<  "data94";
	if (randomSend)
		filename << "random";
	filename<<"_"<<(int)nSensors<<"_"<<(int)nGateways<<"_"<<length<<"_"<<duration-measurementStart<<"_"<<iterationCount<<"_"<<pktsize<<"_"<<interval;
	if(ack)
		filename << "_ack";
	if(rslora)
		filename << "_rslora";
	if(learning)
		filename << "_learning";
	if(optimized)
		filename << "_optimized";
	if(interference)
		filename << "_interference";
	m_stream = ascii.CreateFileStream(filename.str());
	*m_stream->GetStream() << "#Scenario " << (int)nSensors <<  " nodes with " << (int)nGateways << " gateways and " << (int)nSensors << " nodes on a square field with side " << length << " meter" <<std::endl;
	*m_stream->GetStream() << "#ack " << ack << ", pktsize "  << pktsize;
	*m_stream->GetStream() << ", rate " << interval << ", rslora "  << rslora;
	*m_stream->GetStream() << ", learning " << learning << ", optimized "  << optimized;
	*m_stream->GetStream() << ", energy " << monitorEnergy << ", iterations "  << iterationCount<<std::endl;

	*m_stream->GetStream() << "ID, transmitted, received, uniquelyReceived, closestReceived, xCoords, yCoords, avgRetransmission, avgDelay, noAck, acksReceived,timeInSimulation,energyleft << std::endl;
	for (uint8_t iterationI=0;iterationI<iterationCount;iterationI++){
		std::cout << "Iteration: " << (int)iterationI << std::endl;
		mainBody();
		StoreData(duration);
		errorMap.clear();
		Simulator::Destroy ();
	}
	return 0;
}
