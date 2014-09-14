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
#include "ns3/abort.h"

#include <cstdlib>
#include <cstdio>


NS_LOG_COMPONENT_DEFINE ("OwdClient");



static const char* modeNames[] = {
"RTT sampling",
"OWD estimation"
};



OWDHost::OWDHost ()

{
  NS_LOG_FUNCTION (this);
  m_inflight = 0;
//  m_socket = 0;
  m_sendEvent = EventId ();
//  m_count = 1;
//  m_interval = MilliSeconds(2);
  m_highestAcknowledgedAckInRound = 0;
//  m_currentMode = RTTSampling;
  m_sampleRTTmaxRounds = 1;
//  m_txBuffer.SetHeadSequence(1);std::vector<fastestSocket
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
  Ptr<Ipv4> ippeer = m_peer->GetObject<Ipv4>();

  uint32_t nIfs = ip->GetNInterfaces ();
  NS_LOG_INFO("node" << GetNode() << " has " << nIfs << " interfaces ");

  // Counter number of IPs of the peer


  //! for each if create a socket
  // assumes one IP per interface
  for(uint32_t i = 0; i < nIfs ; ++i )
  {
    NS_ASSERT_MSG(ip->GetNAddresses(i ) == 1, "For now assume 1 ip per interface");

      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      Ptr<Socket> sock = Socket::CreateSocket (GetNode (), tid);

      // Skip loopback address
      if( ip->GetAddress(i,0).GetLocal() == Ipv4Address::GetLoopback()) continue;


      // should bind to a specific ip
      // 0 or 1 ?
      NS_LOG_INFO("Socket "<< i << " binding to " << ip->GetAddress(i,0).GetLocal() );
      sock->Bind ( InetSocketAddress ( ip->GetAddress(i,0).GetLocal(), CFG_PORT) );

      // Should be
      sock->SetRecvCallback (MakeCallback( &OWDHost::HandleRecv, this));

      NS_LOG_INFO("Socket "<< i << " connecting to " << ippeer->GetAddress(i,0).GetLocal() );
      sock->Connect( InetSocketAddress(ippeer->GetAddress(i,0).GetLocal() , CFG_PORT) );
      m_sockets.push_back(sock);
  }
  ChangeMode(RTTSampling);

// TODO start with probign RTTs
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &OWDHost::SampleRTTStart, this);
}



void
OWDHost::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  // Close sockets


  Simulator::Cancel (m_sendEvent);
}



void
OWDHost::ChangeMode(Mode mode)
{
  NS_LOG_FUNCTION(this << "From mode " << modeNames[m_currentMode] << " to " << modeNames[mode]);

  // Check that every ACK in flight has been acknowledged

  m_currentMode=mode;

  // Reset current round stats
  RoundStats stats;
  m_currentRoundStats = stats;
}


void
OWDHost::EstimateOWDRecv(int sockId, const SeqTsHeader& seqTs)
{
  NS_LOG_FUNCTION(this);
}


int
OWDHost::GetIndexOfSocket(Ptr<Socket> sock)
{
  //!
  for(int i = 0 ; i < (int)m_sockets.size() ; ++i)
  {
    //!
    if( m_sockets[i] == sock)
      return i;

  }

  NS_FATAL_ERROR("sock should exist !!");
}

//
//OWDHost::EstimateDeltaOWD()
//{
//
//}
//
//
//int
//OWDHost::GetFastForwardSubflowId()
//{
//
//}
//
//int
//OWDHost::GetSlowForwardSubflowId()
//{
//
//}



RoundStats::RoundStats()
{
    //!
  FastestForwardSubflow = -1;
//  RealReverseDeltaOWD;
//  Time EstimatedForwardDeltaOWD = 0;
//  Time RealForwardDeltaOWD = 0;
//  Time EstimatedReverseDeltaOWD = 0;
}



void
OWDHost::HandleRecv( Ptr<Socket> socket )
{
  NS_LOG_FUNCTION (this);

  // TODO depending on the socket
  Ptr<Packet> packet;
  Address from;

  // ce while est bizarre.
  while ((packet = socket->RecvFrom (from)))
    {

      NS_LOG_INFO ("At time " << Simulator::Now ().GetMilliSeconds () << "ms client received " << packet->GetSize () << " bytes from " <<
                   InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                   InetSocketAddress::ConvertFrom (from).GetPort ());

      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

      // Retrieve socket Id
      int id = GetIndexOfSocket( socket );

      // Extract header from packet
      SeqTsHeader seqTs;
      NS_ASSERT(packet->RemoveHeader(seqTs) >= 0);

      NS_LOG_INFO ("At time " << Simulator::Now ().GetMicroSeconds()  << "µs server received packet with TS ["
                   << seqTs.GetTs() << "] and seq [" << seqTs.GetSeq() << "] from "
                   << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
//                   << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                   );

      if(m_currentMode == RTTSampling)
      {
        SamplingRTTRecv(id, seqTs);
      }
      else
      {
        EstimateOWDRecv(id, seqTs);
      }
    }
}


int
OWDHost::GetRoundNo() const
{
  if(m_currentMode == RTTSampling)
  {
    return m_rttRoundStats.size();
  }

  return m_owdRoundStats.size();
}

void
OWDHost::SampleRTTStart(void)
{
  NS_LOG_FUNCTION (this);
//  NS_ASSERT (m_sendEvent.IsExpired ());

  NS_ASSERT( m_currentMode == RTTSampling);
  NS_ASSERT_MSG( m_inflight == 0,"Can't start if packets in flight");


  // Send on all subflows a packet at the same time
  for(int i = 0 ; i < (int)m_sockets.size() ;++i)
  {
    Send( m_sockets[i] , m_inflight);
    ++m_inflight;
  }

  // Random number should be superior to any seq nb previously sent
  m_highestAcknowledgedAckInRound = m_inflight + 4000;
}

//RttRoundStats::RttRoundStats()
//{
//  //!
//}

void
OWDHost::SamplingRTTRecv(int sockId, const SeqTsHeader& seqTs)
{
  NS_LOG_FUNCTION(this);

  m_inflight--;


  // socket index / arrival order at the receiver
  // immediately deduced from received seq nb.
//  m_forwardOrder.push_back( std::make_pair(id,seqTs.GetSeq()) );

  // TODO record the

  //  sample.estimate
  m_currentRoundStats.rtt[sockId ] = TimeStep(Simulator::Now().GetTimeStep() ) - seqTs.GetReceiverTs();
//  m_currentRoundStats.RealForwardDeltaOWD = seqTs.GetSenderTs() - seqTs.GetReceiverTs();
//  m_currentRoundStats.RealReverseDeltaOWD = Simulator::Now().GetTimeStep() - seqTs.GetSenderTs();
  if( seqTs.GetSeq() < m_highestAcknowledgedAckInRound)
  {
    NS_LOG_INFO("Found fastest subflow") ;
    m_highestAcknowledgedAckInRound = seqTs.GetSeq();
    m_currentRoundStats.FastestForwardSubflow = sockId;
  }

  //! if we received echoed packet from all subflows
  if(m_inflight <= 0)
  {
    //! We finished a round => reset
    NS_LOG_INFO("Rtt sampling: finished round " << GetRoundNo() << " (out of " << m_sampleRTTmaxRounds << "). "
      << "Subflow [" << sockId << "] forward delay looks shorter: "
//      << << " < " << ;
      );

    m_rttRoundStats.push_back(m_currentRoundStats);

    //! if we finished enough rounds
    if( GetRoundNo() >= (int)m_sampleRTTmaxRounds)
    {
      // Then we shall change mode to start checking OWDs
      ChangeMode(OWDEstimation);

    }
    return;
  }

  //! Here it means we still expect a seq on another path
}

void
OWDHost::EstimateOWDStart(void)
{
  NS_LOG_FUNCTION (this);
//  NS_ASSERT (m_sendEvent.IsExpired ());

  NS_ASSERT( m_currentMode == OWDEstimation);


  // First we should estimate the OWD from the past samples

  for(int i = 0 ; i < (int)m_sockets.size() ;++i)
  {
//    Ptr<Socket> sock = m_sockets[i];

    Send( m_sockets[i] , m_inflight);
    ++m_inflight;
  }


//  if (m_inflight < m_count)
//    {
//      m_sendEvent = Simulator::Schedule (m_interval, &OWDHost::Send, this);
//    }
}


void
OWDHost::Send(Ptr<Socket> sock, uint32_t seqNb)
{
  NS_LOG_FUNCTION(this << " Sending seq  [" << seqNb << "]");
  NS_ASSERT(sock);

  SeqTsHeader seqTs;
  seqTs.SetSeq (seqNb);

  Address addr;
  sock->GetSockName(addr);

  Ptr<Packet> packet = Create<Packet> ( seqTs.GetSerializedSize() ); // 8+4 : the size of the seqTs header
  packet->AddHeader (seqTs);

  NS_ASSERT_MSG((sock->Send (packet)) >= 0, "Error while sending packet");

  NS_LOG_INFO ( "Send seq nb [" << seqTs.GetSeq() << "] from " << InetSocketAddress::ConvertFrom(addr).GetIpv4()
//               << " to "
               <<" at Time: " <<(Simulator::Now ()).GetMicroSeconds() << "µs "
               " with timestamp [" << seqTs.GetTs() << "]"
               );
}
