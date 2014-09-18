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
#include <sstream>
#include <cmath>


NS_LOG_COMPONENT_DEFINE ("OwdClient");



static const char* modeNames[] = {
"RTT sampling",
"OWD estimation"
};



OWDHost::OWDHost () :
//  m_probesInARound(3),
  m_sampleRTTmaxRounds(3),m_lowestRoundSeq(0)
//  ,m_owdMaxRounds(10)

{
  NS_LOG_FUNCTION (this);
  m_inflight = 0;
//  m_socket = 0;
//  m_sendEvent = EventId ();
//  m_count = 1;
//  m_interval = MilliSeconds(2);
//  m_highestAcknowledgedAckInRound = 0;
  m_forwardFastSubflow = -1; //!<
//  m_probeNo = 0;
//  m_currentMode = RTTSampling;
//   m_sampleRTTmaxRounds = 1;
//   = 3;
//  m_txBuffer.SetHeadSequence(1);std::vector<fastestSocket


}

OWDHost::~OWDHost ()
{
  NS_LOG_FUNCTION (this);
}


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
//  m_sendEvent =
  Simulator::Schedule (Seconds (0.0), &OWDHost::SampleRTTStart, this);
}



void
OWDHost::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  // Close sockets


//  Simulator::Cancel (m_sendEvent);
}



void
OWDHost::ChangeMode(Mode mode)
{
  NS_LOG_FUNCTION(this << "From mode " << modeNames[m_currentMode] << " to " << modeNames[mode]);

  NS_ASSERT(m_inflight );
  // Check that every ACK in flight has been acknowledged

  m_currentMode=mode;

  // Reset current round stats
//  RoundStats stats;
//  m_currentRoundStats = stats;




}


////
//void
//OWDHost::EstimateOWDStartNewRound(int& )
//{
//  NS_LOG_FUNCTION (this);
//  NS_ASSERT( m_currentMode == OWDEstimation);
//  NS_ASSERT( m_inflight == 0);
//
////  Time ForwardDeltaOwd = 0;
//
//
//  ///// First we need to decide which are forward slow & fast path
//  //////////////////////////////////////////////////////
//  if( GetRoundNo() == 0)
//  {
//    // If first round, we decide upon the RTTsampling phase
//    NS_ASSERT( !m_rttRoundStats.empty() );
//
//    m_forwardFastSubflow = m_rttRoundStats.back().ForwardFastSubflow;
//
//    // Rtt slow -  abs(DeltaRTT)/2
////    m_estimatedForwardDeltaOwd = MicroSeconds(std::abs( (m_rttRoundStats.back().rtt[0]- m_rttRoundStats.back().rtt[1]).GetMicroSeconds() ) );
//    m_estimatedForwardDeltaOwd = ns3::Abs( (m_rttRoundStats.back().rtt[0]- m_rttRoundStats.back().rtt[1]) );
//    m_estimatedForwardDeltaOwd = MicroSeconds( m_estimatedForwardDeltaOwd.GetMicroSeconds()/2);
//    NS_LOG_INFO("First OWD estimation round. Setting fast forward path to [" << m_forwardFastSubflow <<"]"
//                << " and setting estimatedForward delay to " << m_estimatedForwardDeltaOwd
//                );
//  }
//  else
//  {
//    NS_ASSERT( !m_owdRoundStats.empty() );
//    //! TODO We should average over the past results
//    m_forwardFastSubflow = m_owdRoundStats.back().ForwardFastSubflow;
//    m_estimatedForwardDeltaOwd = m_owdRoundStats.back().EstimatedForwardDeltaOWD;
//  }
//
//
//  // First we should estimate the OWD from the past samples
//  NS_ASSERT( ForwardSlowSubflowId() >= 0 && ForwardSlowSubflowId() < (int)m_sockets.size() );
//  NS_ASSERT( ForwardFastSubflowId() >= 0 && ForwardFastSubflowId() < (int)m_sockets.size() );
//  Ptr<Socket> slowSocket = m_sockets[ForwardSlowSubflowId()];
//  Ptr<Socket> fastSocket = m_sockets[ForwardFastSubflowId()];
//
//  //! Send seq nb 2 on slow socket
//  Send(slowSocket,1);
////  Simulator::Now()
//
//  //! Then we send the probes on the fast path
////  for(int i= -m_probesInARound/2; i < m_probesInARound/2+m_probesInARound%2; i++)
//  for(uint8_t i= 0; i < m_probesInARound; i++)
//  {
//    //! sent on forward fast socket seq nb "1"
////    Time scheduledTime = m_estimatedForwardDeltaOwd + i*delayBetweenProbes;
//    Time scheduledTime = GetProbeDelay(i);
//    NS_LOG_INFO("Scheduling send on fast forward socket of seqnb 0 in "<< scheduledTime.GetMilliSeconds() << "ms");
//    Simulator::Schedule ( scheduledTime , &OWDHost::Send, this, fastSocket, 0);
//  }
//
//  //! nb of packets sent = nb of probes + packet on slow path
//  m_inflight = m_probesInARound + 1;
//
////  if (m_inflight < m_count)
////    {
////      m_sendEvent = Simulator::Schedule (m_interval, &OWDHost::Send, this);
////    }
//}
//




void
OWDHost::EstimateOWDRecv(int sockId, const SeqTsHeader& seqTs)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_currentMode == OWDEstimation);

//  static probeNo = 0;
//
//  int arrivalPosition = GetNumberOfProbesThisRound() - m_inflight;

  Time timeOfArrival = Simulator::Now();

  if( sockId == ForwardFastSubflowId())
  {
    NewPacketFromFastPath( 0, timeOfArrival, seqTs );
//    m_probeNo++;
  }
  else
  {
    NewPacketFromSlowPath( 0, timeOfArrival, seqTs );
  }


  //! if we received echoed packet from all subflows
  if(m_inflight <= 0)
  {
    RoundStats stats = FinishRound();
    m_owdRoundStats.push_back(stats);


    //! if we finished enough rounds
    if( ReachedConvergence() )
    {
      // Then we have finished the experimentation and we should exit.
      std::stringstream dump;
      DumpOwdSamples(dump);
      NS_LOG_INFO("Dumping OWD samples: \n" << dump.str() );

    }
    else
    {
      // Simulator::Schedule (Seconds (0.0), &OWDHost::SampleRTTStart, this);
      StartNewOwdEstimationRound();
    }
  }

}



void
OWDHost::SamplingRTTRecv(int sockId, const SeqTsHeader& seqTs)
{
  NS_LOG_FUNCTION(this);



  // socket index / arrival order at the receiver
  // immediately deduced from received seq nb.
//  m_forwardOrder.push_back( std::make_pair(id,seqTs.GetSeq()) );

  // TODO record the

  //  sample.estimate
  m_currentRoundStats.rtt[sockId ]             = TimeStep(Simulator::Now().GetTimeStep() ) - seqTs.GetReceiverTs();
  m_currentRoundStats.RealForwardDelay[sockId] = seqTs.GetSenderTs() - seqTs.GetReceiverTs();
  m_currentRoundStats.RealReverseDelay[sockId] = Simulator::Now() - seqTs.GetSenderTs();


  // If ForwardFast subflow not set and we saw the lowest seq nb
  if( seqTs.GetSeq() == m_lowestRoundSeq && m_currentRoundStats.ForwardFastSubflow < 0)
  {
    // Then we update the fast subflow id
    NS_LOG_INFO("Updated fastest subflow id [" << sockId << "]") ;
//    m_highestAcknowledgedAckInRound = seqTs.GetSeq();
    m_currentRoundStats.ForwardFastSubflow = sockId;
    m_forwardFastSubflow = m_currentRoundStats.ForwardFastSubflow;
  }


  //! if we received echoed packet from all subflows
  if(m_inflight <= 0)
  {
    //! We finished a round => reset round
    NS_LOG_INFO("Rtt sampling: finished round " << GetRoundNo() << " (out of " << m_sampleRTTmaxRounds << "). "
      << "Subflow [" << sockId << "] forward delay looks shorter: "
      );

    //! at this stage we can't compute a fast forward subflow

    m_rttRoundStats.push_back(m_currentRoundStats);

    //! if we finished enough rounds
    if( GetRoundNo() >= (int)m_sampleRTTmaxRounds)
    {
      std::stringstream dump;
      DumpRttSamples(dump);
      NS_LOG_INFO("Dumping RTT samples: \n" << dump.str() );

      // Then we shall change mode to start checking OWDs
      ChangeMode(OWDEstimation);

      StartNewOwdEstimationRound();
    }
    else
    {
      //! start a new RTT round
      SampleRTTStart();
    }
  }

  //! Here it means we still expect a seq on another path
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
int
OWDHost::ForwardFastSubflowId() const
{
  return m_forwardFastSubflow;
}
//
int
OWDHost::ForwardSlowSubflowId() const
{
  return (m_forwardFastSubflow + 1)%2;
}



RoundStats::RoundStats()
{
    //!
  ForwardFastSubflow = -1;
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

      --m_inflight;
//      NS_LOG_INFO ("At time " << Simulator::Now ().GetMilliSeconds () << "ms client received packet from "
//                   << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
////                   << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
//                   );

      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

      // Retrieve socket Id
      int sockId = GetIndexOfSocket( socket );

      // Extract header from packet
      SeqTsHeader seqTs;
      NS_ASSERT(packet->RemoveHeader(seqTs) >= 0);

      NS_LOG_INFO ("At time " << Simulator::Now ().GetMilliSeconds()  << "ms client received packet with TS ["
                   << seqTs.GetTs() << "] and seq [" << seqTs.GetSeq()
                   << "] from " << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
//                   << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                   );



      if(m_currentMode == RTTSampling)
      {
        SamplingRTTRecv( sockId, seqTs);
      }
      else
      {
        EstimateOWDRecv( sockId, seqTs);
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


  m_lowestRoundSeq += 2;

  // Send on all subflows a packet at the same time
  for(int i = 0 ; i < (int)m_sockets.size() ;++i)
  {
    // ts + 1 cause every element in a map must be unique
    Send( m_sockets[i] , m_lowestRoundSeq + i,  Simulator::Now().GetTimeStep() + i);
    ++m_inflight;
  }

  // Random number should be superior to any seq nb previously sent
//  m_highestAcknowledgedAckInRound = m_inflight + 4000;
}

//RttRoundStats::RttRoundStats()
//{
//  //!
//}

void
OWDHost::DumpRttSamples(std::ostream& os) const
{
  //!
  NS_LOG_FUNCTION(this);

  //! Describes metadata
  os << "#RoundId Rtt0 rtt1 id(Fastest)" << std::endl;
  for(RoundStatsCollection::const_iterator it = m_rttRoundStats.begin(); it != m_rttRoundStats.end(); it++)
  {
    //!

    os << std::distance(m_rttRoundStats.begin(), it)
      << "," << it->rtt[0].GetMicroSeconds()
      << "," << it->rtt[1].GetMicroSeconds()
      << "," << it->ForwardFastSubflow
      << std::endl;
  }

}



void
OWDHost::DumpOwdSamples(std::ostream& os) const
{
  NS_LOG_FUNCTION(this);

  Time::Unit unit = Time::MS;
  //!
  os << "#roundId rtt0 rtt1 realForwardOWD0 realReverseOWD0 realForwardOWD1 realReverseOWD1 estimatedForwardOWD0 estimatedReverseOwd1 estimatedForwardOWD1 estimatedReverseOwd0 halfRTT0 halfRTT1 id(Fastest)" << std::endl;
  for(RoundStatsCollection::const_iterator it = m_owdRoundStats.begin(); it != m_owdRoundStats.end(); it++)
  {
    //!
    // TODO Display only when they change
    os <<  std::distance(m_owdRoundStats.begin(), it )
      << "," << it->rtt[0].To(unit)
      << "," << it->rtt[1].To(unit)

      << "," << it->RealForwardDelay[0].GetMicroSeconds()
      << "," << it->RealForwardDelay[1].GetMicroSeconds()

      << "," << it->RealReverseDelay[0].GetMicroSeconds()
      << "," << it->RealReverseDelay[1].GetMicroSeconds();

    if(it->ForwardFastSubflow)
    {
      os << "," << it->EstimatedForwardDelay[0]
         << "," << it->EstimatedForwardDelay[1]
         << "," << it->EstimatedForwardDeltaOWD.GetMicroSeconds();
    }
    else
    {
      os << ",,,";
    }


//    if(it->EstimatedReverseDeltaOWD.IsPositive())
//      os <<

    os << "," << (it->rtt[0].GetMicroSeconds()/2)
      << "," << (it->rtt[1].GetMicroSeconds()/2)
      << "," << it->ForwardFastSubflow;

    if(it->ReverseFastSubflow >=0)
    {
      os << ","<< it->EstimatedReverseDeltaOWD.To(unit)
         << "," << it->EstimatedReverseDelay[0].GetMicroSeconds()
         << "," << it->EstimatedReverseDelay[1].GetMicroSeconds()
         << "," << it->ReverseFastSubflow;
    }
    else {
      os << ",,,,";
    }

    os << std::endl;
  }

}





void
OWDHost::Send(Ptr<Socket> sock, uint32_t seqNb
              , uint64_t ts
              )
{
  NS_LOG_FUNCTION(this << " Sending seq  [" << seqNb << "]");
  NS_ASSERT(sock);

  SeqTsHeader seqTs;
  seqTs.SetSeq (seqNb);
  seqTs.SetSenderTs(ts);

  Address addr;
  sock->GetSockName(addr);

  Ptr<Packet> packet = Create<Packet> ( seqTs.GetSerializedSize() ); // 8+4 : the size of the seqTs header
  packet->AddHeader (seqTs);

  NS_ASSERT_MSG((sock->Send (packet)) >= 0, "Error while sending packet");

//  Time placeholder;

  NS_LOG_INFO ( "Send seq nb [" << seqTs.GetSeq() << "] from " << InetSocketAddress::ConvertFrom(addr).GetIpv4()
//               << " to "
//.GetMilliSeconds()
               <<" at Time: " <<(Simulator::Now ()) << ""
               " with timestamp [" << seqTs.GetTs() << "]"
               );
}
