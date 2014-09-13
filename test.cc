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


#include "owd-client.h"
#include "owd-server.h"

#include <string>
#include <limits>

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("OwdMainTest");
#if 0
static void
AddInternetStack (Ptr<Node> node)
{
  //ARP
  Ptr<ArpL3Protocol> arp = CreateObject<ArpL3Protocol> ();
  node->AggregateObject (arp);
  //IPV4
  Ptr<Ipv4L3Protocol> ipv4 = CreateObject<Ipv4L3Protocol> ();
  //Routing for Ipv4
  Ptr<Ipv4ListRouting> ipv4Routing = CreateObject<Ipv4ListRouting> ();
  ipv4->SetRoutingProtocol (ipv4Routing);
  Ptr<Ipv4StaticRouting> ipv4staticRouting = CreateObject<Ipv4StaticRouting> ();
  ipv4Routing->AddRoutingProtocol (ipv4staticRouting, 0);
  node->AggregateObject (ipv4);
  //ICMP
  Ptr<Icmpv4L4Protocol> icmp = CreateObject<Icmpv4L4Protocol> ();
  node->AggregateObject (icmp);
  //UDP
  Ptr<UdpL4Protocol> udp = CreateObject<UdpL4Protocol> ();
  node->AggregateObject (udp);
  //TCP
  Ptr<TcpL4Protocol> tcp = CreateObject<TcpL4Protocol> ();
  node->AggregateObject (tcp);
}


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


void
DifferentiatedPacingForOWDEstimation::ReceivePacket (Ptr<Socket> socket, Ptr<Packet> packet, const Address &from)
{
  m_receivedPacket = packet;
}

void
DifferentiatedPacingForOWDEstimation::ReceivePacket2 (Ptr<Socket> socket, Ptr<Packet> packet, const Address &from)
{
  m_receivedPacket2 = packet;
}

void
DifferentiatedPacingForOWDEstimation::ReceivePkt (Ptr<Socket> socket)
{
  uint32_t availableData;
  availableData = socket->GetRxAvailable ();
  m_receivedPacket = socket->Recv (std::numeric_limits<uint32_t>::max (), 0);
  NS_ASSERT (availableData == m_receivedPacket->GetSize ());
}

void
DifferentiatedPacingForOWDEstimation::ReceivePkt2 (Ptr<Socket> socket)
{
  uint32_t availableData;
  availableData = socket->GetRxAvailable ();
  m_receivedPacket2 = socket->Recv (std::numeric_limits<uint32_t>::max (), 0);
  NS_ASSERT (availableData == m_receivedPacket2->GetSize ());
}

void
DifferentiatedPacingForOWDEstimation::DoSendData (Ptr<Socket> socket, std::string to)
{
  Address realTo = InetSocketAddress (Ipv4Address (to.c_str ()), 1234);
  NS_TEST_EXPECT_MSG_EQ (socket->SendTo (Create<Packet> (123), 0, realTo),
                         123, "100");
}

void
DifferentiatedPacingForOWDEstimation::SendData (Ptr<Socket> socket, std::string to)
{
  m_receivedPacket = Create<Packet> ();
  m_receivedPacket2 = Create<Packet> ();
  Simulator::ScheduleWithContext (socket->GetNode ()->GetId (), Seconds (0),
                                  &DifferentiatedPacingForOWDEstimation::DoSendData, this, socket, to);
  Simulator::Run ();
}



void
DifferentiatedPacingForOWDEstimation::SetupSimulation()
{
  //!
  // Create topology

  // Receiver Node
  m_m_rxNode = CreateObject<Node> ();
  AddInternetStack (m_rxNode);
  // TODO replace by P2P links
  Ptr<SimpleNetDevice> rxDev1, rxDev2;
  { // first interface
    rxDev1 = CreateObject<SimpleNetDevice> ();
    rxDev1->SetAddress (Mac48Address::ConvertFrom (Mac48Address::Allocate ()));
    m_rxNode->AddDevice (rxDev1);
    Ptr<Ipv4> ipv4 = m_rxNode->GetObject<Ipv4> ();
    uint32_t netdev_idx = ipv4->AddInterface (rxDev1);
    Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (Ipv4Address ("10.0.0.1"), Ipv4Mask (0xffff0000U));
    ipv4->AddAddress (netdev_idx, ipv4Addr);
    ipv4->SetUp (netdev_idx);
  }

  { // second interface
    rxDev2 = CreateObject<SimpleNetDevice> ();
    rxDev2->SetAddress (Mac48Address::ConvertFrom (Mac48Address::Allocate ()));
    m_rxNode->AddDevice (rxDev2);
    Ptr<Ipv4> ipv4 = m_rxNode->GetObject<Ipv4> ();
    uint32_t netdev_idx = ipv4->AddInterface (rxDev2);
    Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (Ipv4Address ("10.0.1.1"), Ipv4Mask (0xffff0000U));
    ipv4->AddAddress (netdev_idx, ipv4Addr);
    ipv4->SetUp (netdev_idx);
  }

  // Sender Node
  m_m_txNode  = CreateObject<Node> ();
  AddInternetStack (m_m_txNode  );
  Ptr<SimpleNetDevice> txDev1;
  {
    txDev1 = CreateObject<SimpleNetDevice> ();
    txDev1->SetAddress (Mac48Address::ConvertFrom (Mac48Address::Allocate ()));
    m_txNode ->AddDevice (txDev1);
    Ptr<Ipv4> ipv4 = m_txNode ->GetObject<Ipv4> ();
    uint32_t netdev_idx = ipv4->AddInterface (txDev1);
    Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (Ipv4Address ("10.0.0.2"), Ipv4Mask (0xffff0000U));
    ipv4->AddAddress (netdev_idx, ipv4Addr);
    ipv4->SetUp (netdev_idx);
  }
  Ptr<SimpleNetDevice> txDev2;
  {
    txDev2 = CreateObject<SimpleNetDevice> ();
    txDev2->SetAddress (Mac48Address::ConvertFrom (Mac48Address::Allocate ()));
    m_txNode ->AddDevice (txDev2);
    Ptr<Ipv4> ipv4 = m_txNode ->GetObject<Ipv4> ();
    uint32_t netdev_idx = ipv4->AddInterface (txDev2);
    Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (Ipv4Address ("10.0.1.2"), Ipv4Mask (0xffff0000U));
    ipv4->AddAddress (netdev_idx, ipv4Addr);
    ipv4->SetUp (netdev_idx);
  }


  //PointToPointHelper pointToPoint;
//pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
//pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));


  // link the two nodes
  Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel> ();
  rxDev1->SetChannel (channel1);
  txDev1->SetChannel (channel1);

  Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel> ();
  rxDev2->SetChannel (channel2);
  txDev2->SetChannel (channel2);


  // Create the UDP sockets
//  Ptr<SocketFactory> rxSocketFactory = m_rxNode->GetObject<UdpSocketFactory> ();
//  Ptr<Socket> rxSocket = rxSocketFactory->CreateSocket ();
//  NS_TEST_EXPECT_MSG_EQ (rxSocket->Bind (InetSocketAddress (Ipv4Address ("10.0.0.1"), 1234)), 0, "trivial");
//  rxSocket->SetRecvCallback (MakeCallback (&DifferentiatedPacingForOWDEstimation::ReceivePkt, this));
//
//  Ptr<Socket> rxSocket2 = rxSocketFactory->CreateSocket ();
//  rxSocket2->SetRecvCallback (MakeCallback (&DifferentiatedPacingForOWDEstimation::ReceivePkt2, this));
//  NS_TEST_EXPECT_MSG_EQ (rxSocket2->Bind (InetSocketAddress (Ipv4Address ("10.0.1.1"), 1234)), 0, "trivial");
//
//  Ptr<SocketFactory> txSocketFactory = m_txNode ->GetObject<UdpSocketFactory> ();
//  Ptr<Socket> txSocket = txSocketFactory->CreateSocket ();
//  txSocket->SetAllowBroadcast (true);
}


void
DifferentiatedPacingForOWDEstimation::DoRun (void)
{


  // ------ Now the tests ------------

  // Unicast test
  SendData (txSocket, "10.0.0.1");
  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket->GetSize (), 123, "trivial");
  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket2->GetSize (), 0, "second interface should receive it");

  m_receivedPacket->RemoveAllByteTags ();
  m_receivedPacket2->RemoveAllByteTags ();

  // Simple broadcast test

  SendData (txSocket, "255.255.255.255");
  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket->GetSize (), 123, "trivial");
  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket2->GetSize (), 0, "second socket should not receive it (it is bound specifically to the second interface's address");

  m_receivedPacket->RemoveAllByteTags ();
  m_receivedPacket2->RemoveAllByteTags ();

  // Broadcast test with multiple receiving sockets

  // When receiving broadcast packets, all sockets sockets bound to
  // the address/port should receive a copy of the same packet -- if
  // the socket address matches.
  rxSocket2->Dispose ();
  rxSocket2 = rxSocketFactory->CreateSocket ();
  rxSocket2->SetRecvCallback (MakeCallback (&DifferentiatedPacingForOWDEstimation::ReceivePkt2, this));
  NS_TEST_EXPECT_MSG_EQ (rxSocket2->Bind (InetSocketAddress (Ipv4Address ("0.0.0.0"), 1234)), 0, "trivial");

  SendData (txSocket, "255.255.255.255");
  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket->GetSize (), 123, "trivial");
  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket2->GetSize (), 123, "trivial");

  m_receivedPacket = 0;
  m_receivedPacket2 = 0;

  // Simple Link-local multicast test

//  txSocket->BindToNetDevice (txDev1);
//  SendData (txSocket, "224.0.0.9");
//  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket->GetSize (), 0, "first socket should not receive it (it is bound specifically to the second interface's address");
//  NS_TEST_EXPECT_MSG_EQ (m_receivedPacket2->GetSize (), 123, "recv2: 224.0.0.9");

//  m_receivedPacket->RemoveAllByteTags ();
//  m_receivedPacket2->RemoveAllByteTags ();



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

int main()
{
  // TODO command line helper to choose parameters (topology, transfer intervals, etc...)

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

  // We create the channels first without any IP addressing information
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", TimeValue ( MilliSeconds(2)) );
  NetDeviceContainer dCdCR1 = p2p.Install (nC, nRC1);

  // Later, we add IP addresses.
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iCiRC1 = ipv4.Assign (dCdCR1);


  NetDeviceContainer dSdRS1 = p2p.Install (nS, nRS1);

  ipv4.SetBase ("20.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iSiRS1 = ipv4.Assign (dSdRS1);


  // Then I want to add an asymetric link
  //AddNetworkRouteTo
//  AddAsymetricLink( nRC1, nRS1, TimeValue( MilliSeconds("30ms")), TimeValue( MilliSeconds("100ms")) );
  p2p.SetChannelAttribute ("Delay", TimeValue( MilliSeconds(30)));
  NetDeviceContainer dRC1dRS1_1 = p2p.Install(nRC1, nRS1);

  p2p.SetChannelAttribute ("Delay", TimeValue( MilliSeconds(100)));
  NetDeviceContainer dRC1dRS1_2 = p2p.Install(nRC1, nRS1);

  ipv4.SetBase ("30.0.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iRC1iRS1_1  = ipv4.Assign (dRC1dRS1_1 );

  ipv4.SetBase ("30.0.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iRC1iRS1_2  = ipv4.Assign (dRC1dRS1_2 );


  p2p.EnablePcapAll ("owd");

  Ptr<Ipv4> ipv4C = nC->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4S = nS->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4RC1 = nRC1->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4RS1 = nRS1->GetObject<Ipv4> ();

//  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ipv4StaticRoutingHelper ipv4RoutingHelper;


  //Print Routin Table
//  Ptr<OutputStreamWrapper> routintable = Create<OutputStreamWrapper>("routingtable",std::ios::out);
//  ipv4RoutingHelper.PrintRoutingTableAllAt(Seconds(3),  routintable);
//  NS_LOG_UNCOND(routintable);

  // Create static routes from A to C
  Ptr<Ipv4StaticRouting> rC = ipv4RoutingHelper.GetStaticRouting (ipv4C);
  Ptr<Ipv4StaticRouting> rS = ipv4RoutingHelper.GetStaticRouting (ipv4S);

  Ptr<Ipv4StaticRouting> rRC1 = ipv4RoutingHelper.GetStaticRouting (ipv4RC1);
  Ptr<Ipv4StaticRouting> rRS1 = ipv4RoutingHelper.GetStaticRouting (ipv4RS1);
//  Ptr<Ipv4StaticRouting> rRC2 = ipv4RoutingHelper.GetStaticRouting (ipv4RC1);

  // add route C -> RC1_1  -> RS1_1 -> S
  rC->AddHostRouteTo (Ipv4Address ("20.0.1.1"), Ipv4Address ("10.0.1.2"), 1);
  rRC1->AddHostRouteTo (Ipv4Address ("20.0.1.1"), 2 , 1); // if , metric
//  rRS1->AddHostRouteTo (Ipv4Address ("20.0.1.1"), Ipv4Address ("20.0.1.1"), 1);

  // add route S -> RS1_1  -> RC1_1 -> C
  rS->AddHostRouteTo (Ipv4Address ("10.0.1.1"), Ipv4Address ("20.0.1.2"), 1);
  rRS1->AddHostRouteTo (Ipv4Address ("10.0.1.1"), 3,1);


  // The ifIndex for this outbound route is 1; the first p2p link added
  // 10.1.1.1 = ipC
//  staticRoutingRC1->AddHostRouteTo (Ipv4Address ("10.1.1.1"), Ipv4Address ("10.1.1.2"), 1);
//  staticRoutingRC1->AddHostRouteTo (Ipv4Address ("20.1.1.1"), Ipv4Address ("30.1.1.2"), 2);
//
//
//  staticRoutingRS1->AddHostRouteTo (Ipv4Address ("10.1.1.1"), Ipv4Address ("30.1.1.1"), 2);
  // Now I want to set the routing tables

//  AddInternetStack (txNode);
//  AddInternetStack (rxNode);

  // Then Create topology, then install application
//  CreateTopology(txNode,rxNode);


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

  Simulator::Run ();
  Simulator::Destroy ();
//  Time::SetResolution (Time::NS);
  NS_LOG_UNCOND("test");
  return 0;
}
