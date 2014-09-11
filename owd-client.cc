/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4.h"

#include "owd-client.h"
#include "ns3/seq-ts-header.h"

#include <cstdlib>
#include <cstdio>


NS_LOG_COMPONENT_DEFINE ("OwdClient");

OWDHost::OWDHost ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
//  m_socket = 0;
  m_sendEvent = EventId ();
}

OWDHost::~OWDHost ()
{
  NS_LOG_FUNCTION (this);
}

//void
//OWDHost::SetRemote (Ipv4Address ip, uint16_t port)
//{
//  NS_LOG_FUNCTION (this << ip << port);
//  m_peerAddress = Address(ip);
//  m_peerPort = port;
//}
//
//OWDHost::SetRemote (uint16_t id, InetSocketAddress)
//{
//
//}

void
OWDHost::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}


void
OWDHost::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  //! count the number of IPs of the node, then it create & bind one socket per IP
  Ptr<Ipv4> ip = GetNode()->GetObject<Ipv4>();

  uint32_t nIfs = ip->GetNInterfaces ();
  NS_LOG_INFO("node" << GetNode() << " has " << nIfs << " interfaces ");

  //! for each if create an IP
  for(uint32_t i = 0; i < nIfs ; ++i )
  {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      Ptr<Socket> sock = Socket::CreateSocket (GetNode (), tid);
      // should bind to a specific ip
      // 0 or 1 ?
      sock->Bind ( InetSocketAddress ( ip->GetAddress(i,0).GetLocal(), CFG_PORT) );

      // Should be
      sock->SetRecvCallback (MakeCallback( &OWDHost::HandleRecv, this));

      m_sockets.push_back(sock);
  }
//  if (m_socket == 0)
//    {
//  TODO ? useless ?
//      m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
//
//    }
  NS_ASSERT( m_sockets.size() == nIfs );

// TODO start with probign RTTs
//  m_sendEvent = Simulator::Schedule (Seconds (0.0), &OWDHost::Send, this);
}

void
OWDHost::HandleRecv( Ptr<Socket> packet )
{
  NS_LOG_FUNCTION (this);

}



void
OWDHost::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
OWDHost::Send (void)
{
//  NS_LOG_FUNCTION (this);
//  NS_ASSERT (m_sendEvent.IsExpired ());
//  SeqTsHeader seqTs;
//  seqTs.SetSeq (m_sent);
//  Ptr<Packet> p = Create<Packet> (m_size-(8+4)); // 8+4 : the size of the seqTs header
//  p->AddHeader (seqTs);
//
//  std::stringstream peerAddressStringStream;
//
//      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
//
//
//  if ((m_socket->Send (p)) >= 0)
//    {
//      ++m_sent;
//      NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
//                                    << peerAddressStringStream.str () << " Uid: "
//                                    << p->GetUid () << " Time: "
//                                    << (Simulator::Now ()).GetSeconds ());
//
//    }
//  else
//    {
//      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
//                                          << peerAddressStringStream.str ());
//    }
//
//  if (m_sent < m_count)
//    {
//      m_sendEvent = Simulator::Schedule (m_interval, &OWDHost::Send, this);
//    }
}


