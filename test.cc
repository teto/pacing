/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

Made by Matthieu Coudron
matthieu.coudron@lip6.fr


Je dois utiliser le udp-echo-server qui retourne tous les paquets qu'on lui envoie. Non il doit
 */

#include "ns3/core-module.h"

//#include "ns3/test.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/socket.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-stack-helper.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"

#include "ns3/arp-l3-protocol.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv6-list-routing.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/gnuplot-helper.h"


#include "owd-client.h"
#include "owd-server.h"

#include <string>
#include <limits>

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("OwdMainTest");
#if 0

class DifferentiatedPacingForOWDEstimation
//: public TestCase
{

public:

  virtual void SetupSimulation();
  virtual void DoRun (void);
  DifferentiatedPacingForOWDEstimation ();
  virtual ~DifferentiatedPacingForOWDEstimation ();

  void ReceivePacket (Ptr<Socket> socket, Ptr<Packet> packet, const Address &from);
  void ReceivePacket2 (Ptr<Socket> socket, Ptr<Packet> packet, const Address &from);
  void ReceivePkt (Ptr<Socket> socket);
  void ReceivePkt2 (Ptr<Socket> socket);

  Ptr<Node> m_rxNode; //

  Ptr<Node> m_txNode ;
  bool m_expectingAllAcks;  //!<

protected:
  Ptr<Packet> m_receivedPacket;
  Ptr<Packet> m_receivedPacket2;
  void DoSendData (Ptr<Socket> socket, std::string to);
  void SendData (Ptr<Socket> socket, std::string to);

};


DifferentiatedPacingForOWDEstimation::DifferentiatedPacingForOWDEstimation ()
{
  NS_LOG_FUNCTION(this);
}


DifferentiatedPacingForOWDEstimation::~DifferentiatedPacingForOWDEstimation ()
{
  NS_LOG_FUNCTION(this);
}

#endif


/**
Asymetric/symetric etc...

1st topology:

    Link A
  __________
 /          \
C            S
 \__________/
    Link B


This is equivalent to
       __________
      /          \
  __RC1__________RS1_
 /                   \
C                     S
 \__RC2________RS2___/
       \_______/

**/


void
CreateTopology(Ptr<Node> a,Ptr<Node> b)
{
  // TODO
  // Should set data rates on netdevices to simulate asymetric links
  PointToPointHelper p2phelper;
  Ipv4AddressHelper iphelper;
  iphelper.SetBase("10.0.0.1", Ipv4Mask("255.255.255.0") );

//  helper.SetChannelAttribute()
//  helper.SetDeviceAttribute()
//CreateObject
//CreateObject

// csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  p2phelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));

  // create devices & link A
//  NetDeviceContainer devices1 = p2phelper.Install(c,s);
//  iphelper.Assign( devices1 );
//
//  // create devices & link B
//  NetDeviceContainer devices2 = p2phelper.Install(c,s);
//  iphelper.SetBase("20.0.0.1", Ipv4Mask("255.255.255.0") );
//  iphelper.Assign( devices2 );



//  Ptr<Ipv4> ipv4 = m_txNode ->GetObject<Ipv4> ();
//  uint32_t netdev_idx = ipv4->AddInterface (devices1. );
//  Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (Ipv4Address ("10.0.1.2"), Ipv4Mask (0xffff0000U));
//  ipv4->AddAddress (netdev_idx, ipv4Addr);
//  ipv4->SetUp (netdev_idx);

  // Here i should set ips to


// NetDeviceContainer ndc = csma.Install (net);

}

/**
a b should be the 2 nodes
**/
void
AddAsymetricLink(Ptr<Node> r1, Ptr<Node> r2, TimeValue r1r2Delay,TimeValue r2r1Delay)
{
  static int subdomain = 0;
  PointToPointHelper p2phelper;
  Ipv4AddressHelper iphelper;
  TimeValue delays[2] = {
    r1r2Delay,
    r2r1Delay
//    StringValue(r1r2Delay),
//    StringValue(r2r1Delay)
  };


//  NodeContainer c = NodeContainer (nA, nB, nC);

  subdomain++;

  // Attribution of the IP addresses

  for(int i =0; i < 2; ++i)
  {

    std::stringstream netAddr;
    netAddr << "30." << (subdomain) << ".0";
    iphelper.SetBase( netAddr.str().c_str(), Ipv4Mask("255.255.255.0") );
    p2phelper.SetChannelAttribute ("Delay", delays[i]);
    NetDeviceContainer container = p2phelper.Install(r1,r2);
    // They don't need to have an IP ?
//    iphelper.Assign(container);
  }

  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  // Create static routes from A to C
//  Ptr<Ipv4StaticRouting> staticRoutingA = ipv4RoutingHelper.GetStaticRouting (ipv4A);
//
//  // The ifIndex for this outbound route is 1; the first p2p link added
//  staticRoutingA->AddHostRouteTo ( Ipv4Address ("192.168.1.1"), Ipv4Address ("10.1.1.2"), 1);
//


}


/**
In our XP rate does not matter

**/
int main()
{
  // TODO command line helper to choose parameters (topology, transfer intervals, etc...)
  CommandLine cmd;
//  cmd.AddValue ("useIpv6", "Use Ipv6", useV6);
//  cmd.Parse (argc, argv);


  Time delayCtoS_1 = MilliSeconds(30);
  Time delayCtoS_2 = MilliSeconds(10);

  Time delayStoC_1 = MilliSeconds(100);
  Time delayStoC_2 = MilliSeconds(200);

//  DifferentiatedPacingForOWDEstimation test;
//  test.SetupSimulation();
//  test.DoRun();
  Ptr<Node> nC    = CreateObject<Node> ();
  Ptr<Node> nRC1 = CreateObject<Node> ();
  Ptr<Node> nRC2 = CreateObject<Node> ();
  Ptr<Node> nS    = CreateObject<Node> ();
  Ptr<Node> nRS1    = CreateObject<Node> ();
  Ptr<Node> nRS2    = CreateObject<Node> ();



  // Install internet stacks
  InternetStackHelper internet;
  internet.Install ( nC );
  internet.Install ( nRC1 );
  internet.Install ( nRC2 );
  internet.Install ( nS );
  internet.Install ( nRS1 );
  internet.Install ( nRS2 );

//  NodeContainer nCnRC1 = NodeContainer;

  // Configure p2p helper
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));

  Ipv4AddressHelper ipv4;


  // We create the channel C<->RC1
  p2p.SetChannelAttribute ("Delay", TimeValue ( MilliSeconds(0)) );
  NetDeviceContainer dCdRC1 = p2p.Install (nC, nRC1);
  NetDeviceContainer dCdRC2 = p2p.Install (nC, nRC2);

  // We create the channels S<->RS1/2
  NetDeviceContainer dSdRS1 = p2p.Install (nS, nRS1);
  NetDeviceContainer dSdRS2 = p2p.Install (nS, nRS2);

  // IP addressing for C / RC1
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iCiRC1 = ipv4.Assign (dCdRC1);

  // IP addressing for C / RC2
  ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iCiRC2 = ipv4.Assign (dCdRC2);



  // IP addressing for S / RS1
  ipv4.SetBase ("20.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iSiRS1 = ipv4.Assign (dSdRS1);

  // IP addressing for S / RS2
  ipv4.SetBase ("20.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iSiRS2 = ipv4.Assign (dSdRS2);


  // Create channels RC1_1<->RS1_1
  p2p.SetChannelAttribute ("Delay", TimeValue( delayCtoS_1 ));
  NetDeviceContainer dRC1dRS1_1 = p2p.Install(nRC1, nRS1);

  // Create channels RC1_2<->RS1_2
  p2p.SetChannelAttribute ("Delay", TimeValue( delayStoC_1  ));
  NetDeviceContainer dRC1dRS1_2 = p2p.Install(nRC1, nRS1);


  // Addressing RC1_1 <-> RS1_1
  ipv4.SetBase ("30.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iRC1iRS1_1  = ipv4.Assign (dRC1dRS1_1 );

  // Addressing RC1_2 <-> RS1_2
  ipv4.SetBase ("30.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iRC1iRS1_2  = ipv4.Assign (dRC1dRS1_2 );


  // Create channels RC2_1<->RS2_1
  p2p.SetChannelAttribute ("Delay", TimeValue( delayCtoS_2 ));
  NetDeviceContainer dRC2dRS2_1 = p2p.Install(nRC2, nRS2);

  // Create channels RC2_2<->RS1_2
  p2p.SetChannelAttribute ("Delay", TimeValue( delayStoC_2  ));
  NetDeviceContainer dRC2dRS2_2 = p2p.Install(nRC2, nRS2);

  // Addressing RC2_1<->RS2_1
  ipv4.SetBase ("40.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iRC2iRS2_1  = ipv4.Assign (dRC2dRS2_1 );

  // Addressing RC2_2 <-> RS2_2
  ipv4.SetBase ("40.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iRC2iRS2_2  = ipv4.Assign (dRC2dRS2_2 );





  // Retrieve IPs
  Ptr<Ipv4> ipv4C = nC->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4S = nS->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4RC1 = nRC1->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4RC2 = nRC2->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4RS1 = nRS1->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4RS2 = nRS2->GetObject<Ipv4> ();

//  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ipv4StaticRoutingHelper ipv4RoutingHelper;


  //Print Routin Table
//  Ptr<OutputStreamWrapper> routintable = Create<OutputStreamWrapper>("routingtable",std::ios::out);
//  ipv4RoutingHelper.PrintRoutingTableAllAt(Seconds(3),  routintable);
//  NS_LOG_UNCOND(routintable);

  // retrieve IP stack for each node
  Ptr<Ipv4StaticRouting> rC = ipv4RoutingHelper.GetStaticRouting (ipv4C);
  Ptr<Ipv4StaticRouting> rS = ipv4RoutingHelper.GetStaticRouting (ipv4S);

  Ptr<Ipv4StaticRouting> rRC1 = ipv4RoutingHelper.GetStaticRouting (ipv4RC1);
  Ptr<Ipv4StaticRouting> rRS1 = ipv4RoutingHelper.GetStaticRouting (ipv4RS1);

  Ptr<Ipv4StaticRouting> rRC2 = ipv4RoutingHelper.GetStaticRouting (ipv4RC2);
  Ptr<Ipv4StaticRouting> rRS2 = ipv4RoutingHelper.GetStaticRouting (ipv4RS2);


  // add route C -> RC1_1  -> RS1_1 -> S
  rC->AddHostRouteTo (Ipv4Address ("20.0.1.1"), Ipv4Address ("10.0.1.2"), 1);
  rRC1->AddHostRouteTo (Ipv4Address ("20.0.1.1"), 2 , 1); // if , metric

  // add route S -> RS1_1  -> RC1_1 -> C
  rS->AddHostRouteTo (Ipv4Address ("10.0.1.1"), Ipv4Address ("20.0.1.2"), 1);
  rRS1->AddHostRouteTo (Ipv4Address ("10.0.1.1"), 3,1);


  // add route C -> RC2_1  -> RS2_1 -> S
  rC->AddHostRouteTo (Ipv4Address ("20.0.2.1"), Ipv4Address ("10.0.2.2"), 2);
  rRC2->AddHostRouteTo (Ipv4Address ("20.0.2.1"), 2, 1); // if , metric

  // add route S -> RS2_1  -> RC2_1 -> C
  rS->AddHostRouteTo (Ipv4Address ("10.0.2.1"), Ipv4Address ("20.0.2.2"), 2);
  rRS2->AddHostRouteTo (Ipv4Address ("10.0.2.1"), 3,1);




  // Register pcap for debug
//  p2p.EnablePcapAll ("owd");

  // Then install application
  Ptr<OWDHost> clientApp = CreateObject<OWDHost>();
  Ptr<OwdServer> serverApp = CreateObject<OwdServer>();

  clientApp->SetPeer( nS );


  nC->AddApplication( clientApp );
  nS->AddApplication( serverApp );

  clientApp->SetStartTime (Seconds (0.0));
  clientApp->SetStopTime (Seconds (4.0));

  serverApp->SetStartTime (Seconds (0.0));
  serverApp->SetStopTime (Seconds (4.0));


  // To debug, ms resolution is easier to read
  // when
//  Time::SetResolution (Time::NS);
  Time::SetResolution (Time::MS);

  Simulator::Run ();


  // Use GnuplotHelper to plot the packet byte count over time
  GnuplotHelper plotHelper;

  // Configure the plot.  The first argument is the file name prefix
  // for the output files generated.  The second, third, and fourth
  // arguments are, respectively, the plot title, x-axis, and y-axis labels
//  plotHelper.ConfigurePlot ("seventh-packet-byte-count",
//                           "Packet Byte Count vs. Time",
//                           "Time (Seconds)",
//                           "Packet Byte Count");

  // Specify the probe type, probe path (in configuration namespace), and
  // probe output trace source ("OutputBytes") to plot.  The fourth argument
  // specifies the name of the data series label on the plot.  The last
  // argument formats the plot by specifying where the key should be placed.
//  plotHelper.PlotProbe (probeName,
//                       probeTrace,
//                       "OutputBytes",
//                       "Packet Byte Count",
//                       GnuplotAggregator::KEY_BELOW);






  Simulator::Destroy ();

  NS_LOG_UNCOND("test");
  return 0;
}
