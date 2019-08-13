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
 * Authors:
 *	Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */
#include "lora-helper.h"
#include <ns3/lora-module.h>
#include <ns3/packet.h>
#include <ns3/mobility-model.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/drop-tail-queue.h>
#include <ns3/log.h>
#include <ns3/double.h>
#include "ns3/names.h"
#include <ns3/random-variable-stream.h>
#include <ns3/pointer.h>

namespace ns3 {


NS_LOG_COMPONENT_DEFINE ("LoRaHelper");

/**
 * @brief Output an ascii line representing the Transmit event (with context)
 * @param stream the output stream
 * @param context the context
 * @param p the packet
 */
static void
AsciiLoRaMacTransmitSinkWithContext (
  Ptr<OutputStreamWrapper> stream,
  std::string context,
  Ptr<const Packet> p)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << " " << *p << std::endl;
}

/**
 * @brief Output an ascii line representing the Transmit event (without context)
 * @param stream the output stream
 * @param p the packet
 */
static void
AsciiLoRaMacTransmitSinkWithoutContext (
  Ptr<OutputStreamWrapper> stream,
  Ptr<const Packet> p)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << *p << std::endl;
}

LoRaHelper::LoRaHelper (void)
{
  m_channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  m_channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  m_channel->SetPropagationDelayModel (delayModel);
	m_spectrumModel = 0;
}

LoRaHelper::~LoRaHelper (void)
{
  m_channel->Dispose ();
  m_channel = 0;
	m_spectrumModel = 0;
}

void
LoRaHelper::EnableLogComponents (void)
{
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnable ("LoRaErrorModel", LOG_LEVEL_ALL);
  LogComponentEnable ("LoRaNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("LoRaNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("LoRaPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LoRaSpectrumSignalParameters", LOG_LEVEL_ALL);
}

/*
std::string
LoRaHelper::LoRaPhyEnumerationPrinter (LoRaPhyEnumeration e)
{
  switch (e)
    {
    case IEEE_802_15_4_PHY_BUSY:
      return std::string ("BUSY");
    case IEEE_802_15_4_PHY_BUSY_RX:
      return std::string ("BUSY_RX");
    case IEEE_802_15_4_PHY_BUSY_TX:
      return std::string ("BUSY_TX");
    case IEEE_802_15_4_PHY_FORCE_TRX_OFF:
      return std::string ("FORCE_TRX_OFF");
    case IEEE_802_15_4_PHY_IDLE:
      return std::string ("IDLE");
    case IEEE_802_15_4_PHY_INVALID_PARAMETER:
      return std::string ("INVALID_PARAMETER");
    case IEEE_802_15_4_PHY_RX_ON:
      return std::string ("RX_ON");
    case IEEE_802_15_4_PHY_SUCCESS:
      return std::string ("SUCCESS");
    case IEEE_802_15_4_PHY_TRX_OFF:
      return std::string ("TRX_OFF");
    case IEEE_802_15_4_PHY_TX_ON:
      return std::string ("TX_ON");
    case IEEE_802_15_4_PHY_UNSUPPORTED_ATTRIBUTE:
      return std::string ("UNSUPPORTED_ATTRIBUTE");
    case IEEE_802_15_4_PHY_READ_ONLY:
      return std::string ("READ_ONLY");
    case IEEE_802_15_4_PHY_UNSPECIFIED:
      return std::string ("UNSPECIFIED");
    default:
      return std::string ("INVALID");
    }
}*/

/*
std::string
LoRaHelper::LoRaMacStatePrinter (LoRaMacState e)
{
  switch (e)
    {
    case MAC_IDLE:
      return std::string ("MAC_IDLE");
    case CHANNEL_ACCESS_FAILURE:
      return std::string ("CHANNEL_ACCESS_FAILURE");
    case CHANNEL_IDLE:
      return std::string ("CHANNEL_IDLE");
    case SET_PHY_TX_ON:
      return std::string ("SET_PHY_TX_ON");
    default:
      return std::string ("INVALID");
    }
}*/

void
LoRaHelper::AddMobility (Ptr<LoRaPhy> phy, Ptr<MobilityModel> m)
{
  phy->SetMobility (m);
}

NetDeviceContainer
LoRaHelper::Install (NodeContainer c)
{
  NetDeviceContainer devices;
	//remove first MAC address, it is reserved for base stations.
	Mac32Address::Allocate();
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
		Ptr<Node> nodeI = *i;
		Ptr<LoRaNetDevice> anandi = CreateObject<LoRaNetDevice> ();
		devices.Add(anandi);
		Ptr<LoRaPhy> sfp = CreateObject<LoRaPhy> ();
		if (m_spectrumModel == 0)
			m_spectrumModel = sfp->GetRxSpectrumModel();
		else
			sfp->SetRxSpectrumModel (m_spectrumModel);
		anandi->SetPhy (sfp);
		anandi->SetChannel (m_channel);
		anandi->SetAddress(Mac32Address::Allocate());
		Ptr<Queue<QueueItem>> queue = Create<DropTailQueue<QueueItem>>();
		queue->SetMaxPackets(100);
		anandi->SetQueue(queue);
		sfp->SetDevice(anandi);
		sfp->SetMobility (nodeI->GetObject<MobilityModel> ());
		sfp->SetChannel (m_channel);
		sfp->SetRxAntenna (Create<IsotropicAntennaModel> ());
		nodeI->AddDevice(anandi);
		anandi->SetGenericPhyTxStartCallback (MakeCallback(&LoRaPhy::StartTx,sfp));
		sfp->SetTransmissionEndCallback( MakeCallback(&LoRaNetDevice::NotifyTransmissionEnd,anandi));
		sfp->SetReceptionEndCallback ( MakeCallback(&LoRaNetDevice::NotifyReceptionEndOk,anandi));
		sfp->SetReceptionErrorCallback ( MakeCallback(&LoRaNetDevice::NotifyReceptionEndError,anandi));
		sfp->SetReceptionStartCallback ( MakeCallback(&LoRaNetDevice::NotifyReceptionStart,anandi));
		sfp->SetReceptionMacCallback (MakeCallback(&LoRaNetDevice::CheckCorrectReceiver,anandi));
    for (std::list<callbacktuple>::iterator it = m_callbacks.begin(); it!= m_callbacks.end();it++)
    	{
  			anandi->TraceConnectWithoutContext(std::get<0>(*it),std::get<1>(*it));
    	}
    }
  return devices;
}

NetDeviceContainer
LoRaHelper::InstallRs (NodeContainer c)
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
		Ptr<Node> nodeI = *i;
		Ptr<LoRaRsNetDevice> anandi = CreateObject<LoRaRsNetDevice> ();
		devices.Add(anandi);
		Ptr<LoRaPhy> sfp = CreateObject<LoRaPhy> ();
		if (m_spectrumModel == 0)
			m_spectrumModel = sfp->GetRxSpectrumModel();
		else
			sfp->SetRxSpectrumModel (m_spectrumModel);
		anandi->SetPhy (sfp);
		anandi->SetChannel (m_channel);
		anandi->SetAddress(Mac32Address::Allocate());
		Ptr<Queue<QueueItem>> queue = Create<DropTailQueue<QueueItem>>();
		queue->SetMaxPackets(100);
		anandi->SetQueue(queue);
		sfp->SetDevice(anandi);
		sfp->SetMobility (nodeI->GetObject<MobilityModel> ());
		sfp->SetChannel (m_channel);
		sfp->SetRxAntenna (Create<IsotropicAntennaModel> ());
		nodeI->AddDevice(anandi);
		anandi->SetGenericPhyTxStartCallback (MakeCallback(&LoRaPhy::StartTx,sfp));
		sfp->SetTransmissionEndCallback( MakeCallback(&LoRaRsNetDevice::NotifyTransmissionEnd,anandi));
		sfp->SetReceptionEndCallback ( MakeCallback(&LoRaRsNetDevice::NotifyReceptionEndOk,anandi));
		sfp->SetReceptionErrorCallback ( MakeCallback(&LoRaRsNetDevice::NotifyReceptionEndError,anandi));
		sfp->SetReceptionStartCallback ( MakeCallback(&LoRaRsNetDevice::NotifyReceptionStart,anandi));
		sfp->SetReceptionMacCallback (MakeCallback(&LoRaRsNetDevice::CheckCorrectReceiver,anandi));
    for (std::list<callbacktuple>::iterator it = m_callbacks.begin(); it!= m_callbacks.end();it++)
    	{
  			anandi->TraceConnectWithoutContext(std::get<0>(*it),std::get<1>(*it));
    	}
    }
  return devices;
}

NetDeviceContainer
LoRaHelper::InstallGateways (NodeContainer c)
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
  	Ptr<Node> nodeJ = *i;
  	Ptr<LoRaGwNetDevice> anand = CreateObject<LoRaGwNetDevice> ();
  	Ptr<LoRaGwPhy> sfp = CreateObject<LoRaGwPhy> ();
		if (m_spectrumModel == 0)
			m_spectrumModel = sfp->GetRxSpectrumModel();
		else
			sfp->SetRxSpectrumModel (m_spectrumModel);
  	anand->SetPhy (sfp);
  	devices.Add(anand);
  	anand->SetChannel (m_channel);
  	sfp->SetDevice(anand);
  	sfp->SetMobility (nodeJ->GetObject<MobilityModel> ());
  	sfp->SetChannel (m_channel);
  	sfp->SetRxAntenna (Create<IsotropicAntennaModel> ());
		Ptr<Queue<QueueItem>> queue = Create<DropTailQueue<QueueItem>>();
		queue->SetMaxPackets(100);
		anand->SetQueue(queue);
  	nodeJ->AddDevice(anand);
  	anand->SetGenericPhyTxStartCallback (MakeCallback(&LoRaGwPhy::StartTx,sfp));
  	sfp->SetTransmissionEndCallback( MakeCallback(&LoRaGwNetDevice::NotifyTransmissionEnd,anand));
  	sfp->SetReceptionEndCallback ( MakeCallback(&LoRaGwNetDevice::NotifyReceptionEndOk,anand));
  	sfp->SetReceptionStartCallback ( MakeCallback(&LoRaGwNetDevice::NotifyReceptionStart,anand));
  	sfp->SetReceptionErrorCallback ( MakeCallback(&LoRaGwNetDevice::NotifyReceptionEndError,anand));
    for (std::list<callbacktuple>::iterator it = m_gatewayCallbacks.begin(); it!= m_gatewayCallbacks.end();it++)
    {
  		anand->TraceConnectWithoutContext(std::get<0>(*it),std::get<1>(*it));
    }
  }
	return devices;
  // connect gateways to backbone
}

NetDeviceContainer
LoRaHelper::InstallRsGateways (NodeContainer c)
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
  {
  	Ptr<Node> nodeJ = *i;
  	Ptr<LoRaRsGwNetDevice> anand = CreateObject<LoRaRsGwNetDevice> ();
  	Ptr<LoRaGwPhy> sfp = CreateObject<LoRaGwPhy> ();
		if (m_spectrumModel == 0)
			m_spectrumModel = sfp->GetRxSpectrumModel();
		else
			sfp->SetRxSpectrumModel (m_spectrumModel);
  	anand->SetPhy (sfp);
  	devices.Add(anand);
  	anand->SetChannel (m_channel);
  	sfp->SetDevice(anand);
  	sfp->SetMobility (nodeJ->GetObject<MobilityModel> ());
  	sfp->SetChannel (m_channel);
  	sfp->SetRxAntenna (Create<IsotropicAntennaModel> ());
		Ptr<Queue<QueueItem>> queue = Create<DropTailQueue<QueueItem>>();
		queue->SetMaxPackets(100);
		anand->SetQueue(queue);
  	nodeJ->AddDevice(anand);
  	anand->SetGenericPhyTxStartCallback (MakeCallback(&LoRaGwPhy::StartTx,sfp));
  	sfp->SetTransmissionEndCallback( MakeCallback(&LoRaRsGwNetDevice::NotifyTransmissionEnd,anand));
  	sfp->SetReceptionEndCallback ( MakeCallback(&LoRaRsGwNetDevice::NotifyReceptionEndOk,anand));
  	sfp->SetReceptionStartCallback ( MakeCallback(&LoRaRsGwNetDevice::NotifyReceptionStart,anand));
  	sfp->SetReceptionErrorCallback ( MakeCallback(&LoRaRsGwNetDevice::NotifyReceptionEndError,anand));
    for (std::list<callbacktuple>::iterator it = m_gatewayCallbacks.begin(); it!= m_gatewayCallbacks.end();it++)
    {
  		anand->TraceConnectWithoutContext(std::get<0>(*it),std::get<1>(*it));
    }
  }
	return devices;
}



void 
LoRaHelper::AddCallbacks (std::string traceSource, CallbackBase callback)
{
	m_callbacks.push_back(std::make_tuple(traceSource,callback));
}

void 
LoRaHelper::AddCallbacksGateway (std::string traceSource, CallbackBase callback)
{
	m_gatewayCallbacks.push_back(std::make_tuple(traceSource,callback));
}

Ptr<SpectrumChannel>
LoRaHelper::GetChannel (void)
{
  return m_channel;
}

void
LoRaHelper::SetChannel (Ptr<SpectrumChannel> channel)
{
  m_channel = channel;
}

void
LoRaHelper::SetChannel (std::string channelName)
{
  Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel> (channelName);
  m_channel = channel;
}


int64_t
LoRaHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<LoRaNetDevice> LoRa = DynamicCast<LoRaNetDevice> (netDevice);
      if (LoRa)
        {
          //currentStream += LoRa->AssignStreams (currentStream);
        }
    }
  return (currentStream - stream);
}

/**
 * @brief Write a packet in a PCAP file
 * @param file the output file
 * @param packet the packet
 */
static void
PcapSniffLoRa (Ptr<PcapFileWrapper> file, Ptr<const Packet> packet)
{
  file->Write (Simulator::Now (), packet);
}

void
LoRaHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  NS_LOG_FUNCTION (this << prefix << nd << promiscuous << explicitFilename);
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.
  //

  // In the future, if we create different NetDevice types, we will
  // have to switch on each type below and insert into the right
  // NetDevice type
  //
  Ptr<LoRaNetDevice> device = nd->GetObject<LoRaNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("LoRaHelper::EnablePcapInternal(): Device " << device << " not of type ns3::LoRaNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out,
                                                     PcapHelper::DLT_IEEE802_15_4);

  if (promiscuous == true)
    {
      device->TraceConnectWithoutContext ("PromiscSniffer", MakeBoundCallback (&PcapSniffLoRa, file));

    }
  else
    {
      device->TraceConnectWithoutContext ("Sniffer", MakeBoundCallback (&PcapSniffLoRa, file));
    }
}

void
LoRaHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream,
  std::string prefix,
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  Ptr<LoRaNetDevice> device = nd->GetObject<LoRaNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("LoRaHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::LoRaNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      // Ascii traces typically have "+", '-", "d", "r", and sometimes "t"
      // The Mac and Phy objects have the trace sources for these
      //

      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<LoRaNetDevice> (device, "MacRx", theStream);

      device->TraceConnectWithoutContext ("MacTx", MakeBoundCallback (&AsciiLoRaMacTransmitSinkWithoutContext, theStream));

      asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<LoRaNetDevice> (device, "MacTxEnqueue", theStream);
      asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<LoRaNetDevice> (device, "MacTxDequeue", theStream);
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<LoRaNetDevice> (device, "MacTxDrop", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static
  // functions that are always there waiting for just such a case.
  //


  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LoRaNetDevice/MacRx";
  device->TraceConnect ("MacRx", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LoRaNetDevice/Mac/MacTx";
  device->TraceConnect ("MacTx", oss.str (), MakeBoundCallback (&AsciiLoRaMacTransmitSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LoRaNetDevice/Mac/MacTxEnqueue";
  device->TraceConnect ("MacTxEnqueue", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LoRaNetDevice/Mac/MacTxDequeue";
  device->TraceConnect ("MacTxDequeue", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::LoRaNetDevice/Mac/MacTxDrop";
  device->TraceConnect ("MacTxDrop", oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

ApplicationContainer 
LoRaHelper::FinishGateways (NodeContainer nodes, NetDeviceContainer devices, const Address & address)
{
	NS_ASSERT (nodes.GetN () == devices.GetN ());
  ApplicationContainer apps;
	int j = 0;
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); i++)
  {
		apps.Add(FinishGateway (*i, devices.Get(j), address));
		j++;
	}
	return apps;
}

ApplicationContainer 
LoRaHelper::GenerateTraffic(Ptr<RandomVariableStream> var, NodeContainer nodes, int packet_size, double start, double duration, double interval, bool random)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); i++)
    {
		apps.Add(GenerateTraffic (var, *i, packet_size,start,duration,interval,random));
	}
  return apps;
}

Ptr<LoRaNetwork> 
LoRaHelper::InstallBackend (Ptr<Node> node, NetDeviceContainer devices)
{
	Ptr<LoRaNetwork> app = CreateObject<LoRaNetwork> ();
	app->SetStartTime (Seconds (0));
	// install all the network applications
	for (std::list<ObjectFactory>::iterator it = m_netApp.begin(); it!= m_netApp.end(); it++)
	{
		Ptr<LoRaNetworkApplication> netApp = ((*it).Create ())->GetObject<LoRaNetworkApplication> ();
		netApp->SetNetwork (app);
	 	app->TraceConnectWithoutContext("NetRx",MakeCallback(&LoRaNetworkApplication::NewPacket,netApp));
		node->AddApplication (netApp);
	}
  for (NetDeviceContainer::Iterator i = devices.Begin (); i != devices.End (); ++i)
	{
		app->WhiteListDevice ((*i)->GetAddress ());
	}
	node->AddApplication (app);
	return (app);
}

Ptr<Application>
LoRaHelper::FinishGateway (Ptr<Node> node, Ptr<NetDevice> device, const Address & address)
{
  	Ptr<LoRaSinkApplication> app = CreateObject<LoRaSinkApplication>();
		app->SetNetDevice (device);
 		app->SetStartTime (Seconds (0));
  	app->SetAttribute ("ServerAddress", AddressValue(address));
  	//app->SetAttribute ("DataSize",UintegerValue (packet_size));
		node->AddApplication (app);
		return app;
}

Ptr<Application>
LoRaHelper::GenerateTraffic(Ptr<RandomVariableStream> var, Ptr<Node> node, int packet_size, double start, double duration, double interval, bool random)
{
  Ptr<LoRaApplication> app = CreateObject<LoRaApplication>();
  app->SetStartTime (Seconds (start+var->GetValue ()));
  app->SetStopTime (Seconds (start+duration));
  app->SetAttribute ("InterPacketTime",TimeValue( Seconds (interval)));
  app->SetAttribute ("DataSize",UintegerValue (packet_size));
	app->SetAttribute ("RandomSend",BooleanValue(random));
	node->AddApplication (app);
  return app;
}

void
LoRaHelper::InstallNetworkApplication (std::string type, 
                                           std::string n0, const AttributeValue &v0,
                                           std::string n1, const AttributeValue &v1,
                                           std::string n2, const AttributeValue &v2,
                                           std::string n3, const AttributeValue &v3,
                                           std::string n4, const AttributeValue &v4,
                                           std::string n5, const AttributeValue &v5,
                                           std::string n6, const AttributeValue &v6,
                                           std::string n7, const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_netApp.push_back(factory);
}

NodeContainer
LoRaHelper::AddInterference (MobilityHelper helper)
{
	NodeContainer container;
	double lambdas[] = {15,12,45};
	uint32_t fcs[] = {868100000,868300000,868500000};
	// Create one noise file per LoRa Channel
	// this is based on measurements done in Leuven, Belgium
	for (uint32_t j = 0; j<3; j+= 1)
	{
		uint32_t i = fcs[j];
		Ptr<RandomVariableStream> fc = CreateObject<NormalRandomVariable>();
		fc->SetAttribute ("Mean",DoubleValue(i));
		fc->SetAttribute ("Variance",DoubleValue(200));
		Ptr<RandomVariableStream> bandwidth = CreateObject<RandomMixture>();
		Ptr<RandomVariableStream> bandwidth1 = CreateObject<NormalRandomVariable>();
		bandwidth1->SetAttribute ("Mean",DoubleValue(18000));
		bandwidth1->SetAttribute ("Variance",DoubleValue(3e8));
		Ptr<RandomVariableStream> bandwidth2 = CreateObject<NormalRandomVariable>();
		bandwidth2->SetAttribute ("Mean",DoubleValue(115000));
		bandwidth2->SetAttribute ("Variance",DoubleValue(1e8));
		DynamicCast<RandomMixture>(bandwidth)->AddNewDistribution(1,bandwidth1);
		DynamicCast<RandomMixture>(bandwidth)->AddNewDistribution(10,bandwidth2);
		Ptr<RandomMixture> length = CreateObject<RandomMixture>();
		Ptr<RandomVariableStream> length1 = CreateObject<NormalRandomVariable>();//23
		length1->SetAttribute ("Mean",DoubleValue(0.05));
		length1->SetAttribute ("Variance",DoubleValue(8e-7));
		Ptr<RandomVariableStream> length2 = CreateObject<NormalRandomVariable>();//31
		length2->SetAttribute ("Mean",DoubleValue(0.01));
		length2->SetAttribute ("Variance",DoubleValue(5e-7));
		Ptr<RandomVariableStream> length3 = CreateObject<NormalRandomVariable>();//15
		length3->SetAttribute ("Mean",DoubleValue(0.015));
		length3->SetAttribute ("Variance",DoubleValue(8e-7));
		Ptr<RandomVariableStream> length4 = CreateObject<NormalRandomVariable>();//5
		length4->SetAttribute ("Mean",DoubleValue(0.02));
		length4->SetAttribute ("Variance",DoubleValue(11e-7));
		length->AddNewDistribution(23,length1);
		length->AddNewDistribution(31,length2);
		length->AddNewDistribution(15,length3);
		length->AddNewDistribution(5,length4);
		
		for (uint32_t k = 0; k<lambdas[j];k++)
		{
			Ptr<Node> node = CreateObject<Node>();
			// create Noise model
			Ptr<NoiseIsm> noise = CreateObject<NoiseIsm>();
			noise->SetChannel (m_channel);
			Ptr<RandomVariableStream> fc2 = CreateObject<ConstantRandomVariable>();
			fc2->SetAttribute("Constant",DoubleValue(fc->GetValue()));
			noise->SetAttribute("CenterFrequency",PointerValue(fc));
			Ptr<RandomVariableStream> bw2 = CreateObject<ConstantRandomVariable>();
			bw2->SetAttribute("Constant",DoubleValue(bandwidth->GetValue()));
			noise->SetAttribute("Bandwidth",PointerValue(bw2));
			Ptr<RandomVariableStream> length2 = CreateObject<ConstantRandomVariable>();
			length2->SetAttribute("Constant",DoubleValue(length->GetValue()));
			noise->SetAttribute("MessageLength",PointerValue(DynamicCast<RandomVariableStream>(length2)));
			Ptr<RandomVariableStream> time = CreateObject<ExponentialRandomVariable> ();
			time->SetAttribute("Mean",DoubleValue(60));
			noise->SetAttribute("StartTime",PointerValue(time));
			Simulator::Schedule(Seconds(12*60*60+43200),&NoiseIsm::StartNoise,noise);
			node->AggregateObject(noise);
			container.Add(node);
		}
	}
	// and now the "reliable" downlink channel
	Ptr<Node> node = CreateObject<Node>();
	// create Noise model
	Ptr<NoiseIsm> noise = CreateObject<NoiseIsm>();
	noise->SetChannel (m_channel);
	Ptr<RandomVariableStream> fc = CreateObject<UniformRandomVariable>();
	fc->SetAttribute ("Min",DoubleValue(869475000));
	fc->SetAttribute ("Max",DoubleValue(869600000));
	noise->SetAttribute("CenterFrequency",PointerValue(fc));
	Ptr<RandomVariableStream> bandwidth = CreateObject<ExponentialRandomVariable>();
	bandwidth->SetAttribute ("Mean",DoubleValue(9300));
	noise->SetAttribute("Bandwidth",PointerValue(bandwidth));
	Ptr<RandomVariableStream> length = CreateObject<ExponentialRandomVariable>();//23
	length->SetAttribute ("Mean",DoubleValue(0.001));
	noise->SetAttribute("MessageLength",PointerValue(length));
	Ptr<RandomVariableStream> time = CreateObject<ExponentialRandomVariable> ();
	time->SetAttribute("Mean",DoubleValue(18));
	noise->SetAttribute("StartTime",PointerValue(time));
	Simulator::Schedule(Seconds(12*60*60+43200),&NoiseIsm::StartNoise,noise);
	node->AggregateObject(noise);
	container.Add(node);
	helper.Install (container);
	return container;
}

} // namespace ns3

