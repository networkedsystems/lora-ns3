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
 * Authors:
 *  Brecht Reynders <brecht.reynders@esat.kuleuven.be>
 */
#ifndef LORA_HELPER_H
#define LORA_HELPER_H

#include <ns3/node-container.h>
#include <ns3/application-container.h>
#include <ns3/lora-phy.h>
#include <ns3/trace-helper.h>
#include <ns3/callback.h>

namespace ns3 {

class SpectrumChannel;
class MobilityModel;
class RandomVariableStream;
/**
 * \ingroup lora
 *
 * \brief helps to manage and create IEEE 802.15.4 NetDevice objects
 *
 * This class can help to create IEEE 802.15.4 NetDevice objects
 * and to configure their attributes during creation.  It also contains
 * additional helper functions used by client code.
 *
 * Only one channel is created, and all devices attached to it.  If
 * multiple channels are needed, multiple helper objects must be used,
 * or else the channel object must be replaced.
 */

class LoRaHelper : public PcapHelperForDevice,
public AsciiTraceHelperForDevice
{
public:

	/**
	 * \brief Generate constant traffice from dev
	 * \internal
	 *
	 *
	 */

	Ptr<LoRaNetwork> InstallBackend (Ptr<Node> node, NetDeviceContainer devices);
	
	ApplicationContainer GenerateTraffic(Ptr <RandomVariableStream> var, NodeContainer nodes, int packet_size, double start, double duration, double interval);
	Ptr<Application> GenerateTraffic(Ptr <RandomVariableStream> var, Ptr<Node> node, int packet_size, double start, double duration, double interval);


	ApplicationContainer FinishGateways (NodeContainer nodes, NetDeviceContainer devices, const Address & address);
	Ptr<Application> FinishGateway (Ptr<Node> node, Ptr<NetDevice> device, const Address & address);
	/**
   * \brief Create a LoRa helper in an empty state.  By default, a
   * SingleModelSpectrumChannel is created, with a 
   * LogDistancePropagationLossModel and a ConstantSpeedPropagationDelayModel.
   *
   * To change the channel type, loss model, or delay model, the Get/Set
   * Channel methods may be used.
	 */
	LoRaHelper (void);

  /**
   * \brief Create a LoRa helper in an empty state with either a
   * SingleModelSpectrumChannel or a MultiModelSpectrumChannel.
   * \param useMultiModelSpectrumChannel use a MultiModelSpectrumChannel if true, a SingleModelSpectrumChannel otherwise
   *
   * A LogDistancePropagationLossModel and a 
   * ConstantSpeedPropagationDelayModel are added to the channel.
   */

	virtual ~LoRaHelper (void);

	/**
   * \brief Get the channel associated to this helper
   * \returns the channel
   */
  Ptr<SpectrumChannel> GetChannel (void);

  /**
   * \brief Set the channel associated to this helper
   * \param channel the channel
   */
  void SetChannel (Ptr<SpectrumChannel> channel);

  /**
   * \brief Set the channel associated to this helper
   * \param channelName the channel name
   */
  void SetChannel (std::string channelName);

  /**
   * \brief Add mobility model to a physical device
   * \param phy the physical device
   * \param m the mobility model
   */
	void AddMobility (Ptr<LoRaPhy> phy, Ptr<MobilityModel> m);

	/**
	 * \param c a set of nodes
   * \returns A container holding the added net devices.
	 */
	NetDeviceContainer Install (NodeContainer c);
	NetDeviceContainer InstallRs (NodeContainer c);
	
	/**
	 * \param c a set of nodes
     * \returns A container holding the added net devices.
	 */
	NetDeviceContainer InstallGateways (NodeContainer c);
	NetDeviceContainer InstallRsGateways (NodeContainer c);

	void AddCallbacks (std::string traceSource, CallbackBase callback);
	void AddCallbacksGateway (std::string traceSource, CallbackBase callback);

	/**
	 * Helper to enable all LoRa log components with one statement
	 */
	void EnableLogComponents (void);

  /**
   * \brief Transform the LoRaPhyEnumeration enumeration into a printable string.
   * \param e the LoRaPhyEnumeration
   * \return a string
   */
  //static std::string LoRaPhyEnumerationPrinter (LoRaPhyEnumeration e);

  /**
   * \brief Transform the LoRaMacState enumeration into a printable string.
   * \param e the LoRaMacState
   * \return a string
   */
	//static std::string LoRaMacStatePrinter (LoRaMacState e);

	/**
	 * Assign a fixed random variable stream number to the random variables
	 * used by this model. Return the number of streams that have been
	 * assigned. The Install() method should have previously been
	 * called by the user.
	 *
	 * \param c NetDeviceContainer of the set of net devices for which the
	 *          CsmaNetDevice should be modified to use a fixed stream
	 * \param stream first stream index to use
	 * \return the number of stream indices assigned by this helper
	 */
	int64_t AssignStreams (NetDeviceContainer c, int64_t stream);
  
	/**
   * \param type the type of the model to set
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
	 * Install another network application 
   */
  void InstallNetworkApplication (std::string type,
                   std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                   std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                   std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                   std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                   std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                   std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                   std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                   std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

private:
	// Disable implicit constructors
  /**
   * \brief Copy constructor - defined and not implemented.
   */
	LoRaHelper (LoRaHelper const &);
  /**
   * \brief Copy constructor - defined and not implemented.
   * \returns
   */
	LoRaHelper& operator= (LoRaHelper const &);
	/**
	 * \brief Enable pcap output on the indicated net device.
	 *
	 * NetDevice-specific implementation mechanism for hooking the trace and
	 * writing to the trace file.
	 *
	 * \param prefix Filename prefix to use for pcap files.
	 * \param nd Net device for which you want to enable tracing.
	 * \param promiscuous If true capture all possible packets available at the device.
	 * \param explicitFilename Treat the prefix as an explicit filename if true
	 */
	virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);

	/**
	 * \brief Enable ascii trace output on the indicated net device.
	 *
	 * NetDevice-specific implementation mechanism for hooking the trace and
	 * writing to the trace file.
	 *
	 * \param stream The output stream object to use when logging ascii traces.
	 * \param prefix Filename prefix to use for ascii trace files.
	 * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
	 */
	virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream,
			std::string prefix,
			Ptr<NetDevice> nd,
			bool explicitFilename);

  Ptr<SpectrumChannel> m_channel; //!< channel to be used for the devices
	typedef std::tuple<std::string,CallbackBase> callbacktuple;
	std::list<callbacktuple > m_gatewayCallbacks;
  std::list<callbacktuple > m_callbacks;
	std::list<ObjectFactory > m_netApp;  //!< These are the applications installed on the network server
	Ptr<const SpectrumModel> m_spectrumModel;

};

}

#endif /* LORA_HELPER_H */
