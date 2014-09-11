/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "owd-server.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OWDEchoServerApplication");

#if 0
//NS_OBJECT_ENSURE_REGISTERED (UdpEchoServer)
//  ;
//
//TypeId
//UdpEchoServer::GetTypeId (void)
//{
//  static TypeId tid = TypeId ("ns3::UdpEchoServer")
//    .SetParent<Application> ()
//    .AddConstructor<UdpEchoServer> ()
//    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
//                   UintegerValue (9),
//                   MakeUintegerAccessor (&UdpEchoServer::m_port),
//                   MakeUintegerChecker<uint16_t> ())
//  ;
//  return tid;
//}

UdpEchoServer::UdpEchoServer ()
{
  NS_LOG_FUNCTION (this);
}

UdpEchoServer::~UdpEchoServer()
{
  NS_LOG_FUNCTION (this);
//  m_socket = 0;
//  m_socket6 = 0;
}

void
UdpEchoServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
UdpEchoServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  for( int i = 0; i < NUM_INTERFACES; ++i)
  {
    Ptr<Socket> sock = m_sockets[i];

    if (sock == 0)
      {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        sock = Socket::CreateSocket (GetNode (), tid);
        //TODO Should not be any IPv4, must bind to different devices
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
        sock->Bind (local);
//        if (addressUtils::IsMulticast (m_local))
//          {
//            Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (sock);
//            if (udpSocket)
//              {
//                // equivalent to setsockopt (MCAST_JOIN_GROUP)
//                udpSocket->MulticastJoinGroup (0, m_local);
//              }
//            else
//              {
//                NS_FATAL_ERROR ("Error: Failed to join multicast group");
//              }
//          }
      }

      sock->SetRecvCallback (MakeCallback (&UdpEchoServer::HandleRead, this));

      m_socket[i] = sock;
  }


}

void
UdpEchoServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  for( int i = 0; i < NUM_INTERFACES; ++i)
  {
    Ptr<Socket> sock = m_socket[i];
    if (sock != 0)
      {
        sock->Close ();
        sock->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      }
  }
}



void
UdpEchoServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);


  // TODO depending on the socket
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {

      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                   InetSocketAddress::ConvertFrom (from).GetPort ());

      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

      // TODO send lowest received packet
      // change seq paquet
      NS_LOG_LOGIC ("Echoing packet");
      socket->SendTo (packet, 0, from);

      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                   InetSocketAddress::ConvertFrom (from).GetPort ());

    }
}
  #endif
