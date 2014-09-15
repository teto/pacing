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
#include "ns3/ipv4.h"

#include "ns3/seq-ts-header.h"
#include "owd-server.h"
#include "config.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OWDServer");


//NS_OBJECT_ENSURE_REGISTERED (OwdServer)

OwdServer::OwdServer ()
{
  NS_LOG_FUNCTION (this);
}

OwdServer::~OwdServer()
{
  NS_LOG_FUNCTION (this);
//  m_socket = 0;
//  m_socket6 = 0;
}

void
OwdServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
OwdServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  //! count the number of IPs of the node, then it create & bind one socket per IP
  Ptr<Ipv4> ip = GetNode()->GetObject<Ipv4>();


  uint32_t nIfs = ip->GetNInterfaces ();
  NS_LOG_INFO("node" << GetNode() << " has " << nIfs << " interfaces ");


  //! for each if create a socket
  // assumes one IP per interface
  for(uint32_t i = 0; i < nIfs ; ++i )
  {
    NS_ASSERT_MSG(ip->GetNAddresses(i ) == 1, "For now assume 1 ip per interface");

    // skip loopback
    if( ip->GetAddress(i,0).GetLocal() == Ipv4Address::GetLoopback()) continue;

      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      Ptr<Socket> sock = Socket::CreateSocket (GetNode (), tid);
      // should bind to a specific ip
      // 0 or 1 ?
      sock->Bind ( InetSocketAddress ( ip->GetAddress(i,0).GetLocal(), CFG_PORT) );

      // Should be
      sock->SetRecvCallback (MakeCallback( &OwdServer::HandleRead, this));

      m_sockets.push_back(sock);
  }


}

void
OwdServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  for(std::vector< Ptr<Socket> >::iterator i = m_sockets.begin(); i != m_sockets.end() ; ++i)
  {
    (*i)->Close();
    Ptr<Socket> sock = *i;
    sock->Close ();
    sock->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());

  }

}



void
OwdServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);


  // TODO depending on the socket
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
    // Extract header
    SeqTsHeader seqTs;
    NS_ASSERT(packet->RemoveHeader(seqTs) >= 0);

    // update sequencer
    m_sequencer.AckSeqNb( seqTs.GetSeq() );

    NS_LOG_INFO ("At time " << Simulator::Now ().GetMilliSeconds()  << "µs server received packet with sender TS ["
                 << seqTs.GetSenderTs().GetMilliSeconds()
                 << "] and receiver TS [" << seqTs.GetReceiverTs().GetMilliSeconds()
                 << "] and seq [" << seqTs.GetSeq() << "] from "
                 << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
//                   << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                 );

    packet->RemoveAllPacketTags ();
    packet->RemoveAllByteTags ();

    //  Update header value
    seqTs.SetReceiverTs( (uint64_t) seqTs.GetSenderTs().GetTimeStep() );
    seqTs.SetSenderTs( (uint64_t)Simulator::Now ().GetTimeStep () );

    seqTs.SetSeq( m_sequencer.GetHighestRcvdInOrderSeqNb() );
    packet->AddHeader(seqTs);
    socket->SendTo (packet, 0, from);

    NS_LOG_INFO ("At time " << Simulator::Now ().GetMilliSeconds()  << "µs server sends "
                 << " Sender TS " << seqTs.GetSenderTs()
                 << " Receiver TS " << seqTs.GetReceiverTs()

//                 << " from " << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
//                   << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                 );

    }
}
