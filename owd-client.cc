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
  m_probesInARound(3),
  m_sampleRTTmaxRounds(1),
  m_owdMaxRounds(3)

{
  NS_LOG_FUNCTION (this);
  m_inflight = 0;
//  m_socket = 0;
  m_sendEvent = EventId ();
//  m_count = 1;
//  m_interval = MilliSeconds(2);
  m_highestAcknowledgedAckInRound = 0;
  m_forwardFastSubflow = -1; //!<
//  m_currentMode = RTTSampling;
//   m_sampleRTTmaxRounds = 1;
//   = 3;
//  m_txBuffer.SetHeadSequence(1);std::vector<fastestSocket

//  m_estimatedForwardDeltaOwd = 0;
  m_arrivalPositionSlowPacket = -1;
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


  m_arrivalPositionLastProbeBeforeSlowPath = -1;
  m_arrivalPositionFirstProbeAfterSlowPath = -1;

}


//
void
OWDHost::EstimateOWDStartNewRound(void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT( m_currentMode == OWDEstimation);
  NS_ASSERT( m_inflight == 0);

//  Time ForwardDeltaOwd = 0;


  ///// First we need to decide which are forward slow & fast path
  //////////////////////////////////////////////////////
  if( GetRoundNo() == 0)
  {
    // If first round, we decide upon the RTTsampling phase
    NS_ASSERT( !m_rttRoundStats.empty() );

    m_forwardFastSubflow = m_rttRoundStats.back().ForwardFastSubflow;

    // Rtt slow -  abs(DeltaRTT)/2
    m_estimatedForwardDeltaOwd = std::abs( (m_rttRoundStats.back().rtt[0]- m_rttRoundStats.back().rtt[1]).GetMicroSeconds() ) /2.f;
  }
  else
  {
    NS_ASSERT( !m_owdRoundStats.empty() );
    //! TODO We should average over the past results
    m_forwardFastSubflow = m_owdRoundStats.back().ForwardFastSubflow;
    m_estimatedForwardDeltaOwd = m_owdRoundStats.back().EstimatedForwardDeltaOWD;
  }


  // First we should estimate the OWD from the past samples
  Ptr<Socket> slowSocket = m_sockets[ForwardSlowSubflowId()]
  Ptr<Socket> fastSocket = m_sockets[ForwardFastSubflowId()]

  //! Send seq nb 2 on slow socket
  Send(slowSocket,1);
//  Simulator::Now()

  //! Then we send the probes on the fast path
//  for(int i= -m_probesInARound/2; i < m_probesInARound/2+m_probesInARound%2; i++)
  for(uint8_t i= 0; i < m_probesInARound; i++)
  {
    //! sent on forward fast socket seq nb "1"
//    Time scheduledTime = m_estimatedForwardDeltaOwd + i*delayBetweenProbes;
    Time scheduledTime = GetDelayOfProbe(i);
    NS_LOG_INFO("Scheduling send on fast forward socket of seqnb 0 in "<< scheduledTime.GetMilliSeconds() << "ms");
    Simulator::Schedule ( scheduledTime , &OWDHost::Send, this, fastSocket, 0);
  }

  //! nb of packets sent = nb of probes + packet on slow path
  m_inflight = m_probesInARound + 1;

//  if (m_inflight < m_count)
//    {
//      m_sendEvent = Simulator::Schedule (m_interval, &OWDHost::Send, this);
//    }
}



Time
OWDHost::GetDelayOfProbe(uint8_t i) const
{

  /**
  The choice of the delay between probe is really critical for the performance of the algorithm
  (speed of convergence, overhead etc...).
  Here we choose a constant delay for the sake of simplicity but we can imagine many schemes depending
  on the variance of the deltaOWD, the number of available probes (depend on the size of the global/local windows etc...)
  */

  Time delayBetweenProbes = MilliSeconds(5);


  //! need to go into signed mode
  int j = i;
  //! so as to shift j, since we want probes to be centered around DeltaOWD.
  j -= m_probesInARound/2;
  return (m_estimatedForwardDeltaOwd + i*delayBetweenProbes);
}

void
OWDHost::EstimateOWDRecv(int sockId, const SeqTsHeader& seqTs)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_currentMode == OWDEstimation);

  --m_inflight;

  int arrivalPosition = m_probesInARound - inflight;

  // We update the RTT (every probe will overwrite it but it's no problem when delay constant)
  m_currentRoundStats.rtt[sockId ] = TimeStep(Simulator::Now().GetTimeStep() ) - seqTs.GetReceiverTs();

  // This is real because clocks of client & server are synchronized
  m_currentRoundStats.RealForwardDeltaOWD[sockId] = seqTs.GetSenderTs() - seqTs.GetReceiverTs();
  m_currentRoundStats.RealReverseDeltaOWD[sockId] = Simulator::Now().GetTimeStep() - seqTs.GetSenderTs();

  // If
  if( sockId == ForwardSlowSubflowId())
  {
    // If arrived first then it will be 0, if last, it will be m_probesInARound
    m_arrivalPositionSlowPacket = arrivalPosition;

    // Update the
//    if( seqTs.GetSeq() == 0)
//    {
//      /**
//      means packet on slow path arrives before all probes, thus we should reduce the DeltaOWD.
//      Same as for the interval choice between probes, this is critical to the algorithm.
//
//      Here we decrease by 10% the value of the estimated forward deltaOwd.
//      **/
//      m_estimatedForwardDeltaOwd -= 0.1*m_estimatedForwardDeltaOwd;
//    }
//    else
//    {
//      m_highestAcknowledgedAckInRound = 1;
//    }

  }
  //! Packet received from fast subflow
  else
  {
    NS_ASSERT( sockId == ForwardFastSubflowId() );

    //! Here we want to find the probe that arrived at the server last before the first packet on slow path
    //! so
    if(seqTs.GetSeq() == 0)
    {
      NS_LOG_INFO("Probe arrived at the server before slow path packet");
      m_arrivalPositionLastProbeBeforeSlowPath = arrivalPosition;
    }
    else
    {
      NS_ASSERT(seqTs.GetSeq() == 1);
      if(m_arrivalPositionFirstProbeAfterSlowPath < 0)
      {
        NS_LOG_INFO("First probe arrived at the server after slow path packet");
        m_arrivalPositionFirstProbeAfterSlowPath = arrivalPosition;
      }
    }
  }


//  if( seqTs.GetSeq() < m_highestAcknowledgedAckInRound)
//  {
//    NS_LOG_INFO("Updated fastest subflow id [" << sockId << "]") ;
//    m_highestAcknowledgedAckInRound = seqTs.GetSeq();
//    m_currentRoundStats.ForwardFastSubflow = sockId;
//  }
//


  //! if we received echoed packet from all subflows
  if(m_inflight <= 0)
  {


    /**  TODO need now to update OWD estimation
    */

    /* If all probes arrived afterreception of packet on slow path, then it means
    we waited too long before sending probes, ie we overestimated the ForwardDeltaOWD
    */
    if(m_arrivalPositionLastProbeBeforeSlowPath < 0)
    {
      /*
      Same as for the interval choice between probes, the reduction logic is critical to the algorithm
      performance

      As a rule of thumb (should improve it) we decrease by 10% the value of the estimated forward deltaOwd.
      */
      NS_LOG_INFO("All probes arrived after packet on slow path");
      m_estimatedForwardDeltaOwd -= 0.1*m_estimatedForwardDeltaOwd;

      // in this case we can't deduce the ReverseDeltaOWD so we copy the one from previous sampling ?
    }
    /* this is the opposite here, all probes arrived before packet on slow path, ie we underestimated the DeltaOWD */
    else if(m_arrivalPositionFirstProbeAfterSlowPath < 0)
    {
      NS_LOG_INFO("All probes arrived before packet on slow path");
      m_estimatedForwardDeltaOwd += 0.1*m_estimatedForwardDeltaOwd;
    }
    else
    {
      /* Packet on slow path arrived between 2 probes so we can correctly update the DeltaOWD */
      //! Only the arrival between probes interest us so we have to decrease by one position
      //! if packet on slow path arrived before
      if(m_arrivalPositionSlowPacket <m_arrivalPositionLastProbeBeforeSlowPath ){
        --m_arrivalPositionLastProbeBeforeSlowPath;
      }

      NS_LOG_INFO("Probing succeeded in cornering OWD ");

      //!
      m_estimatedForwardDeltaOwd = GetDelayOfProbe(m_arrivalPositionLastProbeBeforeSlowPath);
    }

        //! We finished a round => reset
    NS_LOG_INFO("Owd round " << GetRoundNo() << " finished  (out of " << m_owdMaxRounds << "). "
      << "Subflow [" << sockId << "] forward delay looks shorter: "
      );

    //! TODO we

    m_currentRoundStats.EstimatedForwardDeltaOWD = m_estimatedForwardDeltaOwd;
    m_owdRoundStats.push_back(m_currentRoundStats);
    m_arrivalPositionLastProbeBeforeSlowPath = -1;
    m_arrivalPositionFirstProbeAfterSlowPath = -1;
    m_arrivalPositionSlowPacket = -1;

    //! if we finished enough rounds
    if( GetRoundNo() >= (int)m_owdMaxRounds)
    {
      // Then we have finished the experimentation and we should exit.

      std::stringstream dump;
      DumpOwdSamples(dump);
      NS_LOG_INFO("Dumping OWD samples: \n" << dump.str() );
    }
    return;
  }

}



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
    NS_LOG_INFO("Updated fastest subflow id [" << sockId << "]") ;
    m_highestAcknowledgedAckInRound = seqTs.GetSeq();
    m_currentRoundStats.FastestForwardSubflow = sockId;
  }

  //! if we received echoed packet from all subflows
  if(m_inflight <= 0)
  {
    //! We finished a round => reset round
    NS_LOG_INFO("Rtt sampling: finished round " << GetRoundNo() << " (out of " << m_sampleRTTmaxRounds << "). "
      << "Subflow [" << sockId << "] forward delay looks shorter: "
//      << << " < " << ;
      );

    m_arrivalPositionSlowPacket = 0;
    m_rttRoundStats.push_back(m_currentRoundStats);

    //! if we finished enough rounds
    if( GetRoundNo() >= (int)m_sampleRTTmaxRounds)
    {
      // Then we shall change mode to start checking OWDs
      ChangeMode(OWDEstimation);

      std::stringstream dump;
      DumpRttSamples(dump);
      NS_LOG_INFO("Dumping RTT samples: \n" << dump.str() );
    }
    return;
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
//
//      NS_LOG_INFO ("At time " << Simulator::Now ().GetMilliSeconds () << "ms client received packet from "
//                   << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
////                   << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
//                   );

      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

      // Retrieve socket Id
      int id = GetIndexOfSocket( socket );

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
OWDHost::DumpRttSamples(std::ostream& os) const
{
  //!
  NS_LOG_FUNCTION(this);

  //! Describes metadata
  os << "#RoundId Rtt0 rtt1 id(Fastest)" << std::endl;
  for(RoundStatsCollection::const_iterator it = m_rttRoundStats.begin(); it != m_rttRoundStats.end(); it++)
  {
    //!

    os << std::distance(it, m_rttRoundStats.begin() )
      << " " << it->rtt[0].GetMicroSeconds()
      << " " << it->rtt[1].GetMicroSeconds()
      << " " << it->FastestForwardSubflow
      << std::endl;
  }

}



void
OWDHost::DumpOwdSamples(std::ostream& os) const
{
  NS_LOG_FUNCTION(this);

  //!
  os << "#roundId rtt0 rtt1 realForwardOWD0 realReverseOWD0 realForwardOWD1 realReverseOWD1 estimatedForwardOWD0 estimatedReverseOwd1 estimatedForwardOWD1 estimatedReverseOwd0 halfRTT0 halfRTT1 id(Fastest)" << std::endl;
  for(RoundStatsCollection::const_iterator it = m_owdRoundStats.begin(); it != m_owdRoundStats.end(); it++)
  {
    //!
    os <<  std::distance(it, m_rttRoundStats.begin() )
      << " " << it->rtt[0].GetMicroSeconds()
      << " " << it->rtt[1].GetMicroSeconds()
      << " " << it->RealForwardDeltaOWD[0].GetMicroSeconds()
      << " " << it->RealReverseDeltaOWD[0].GetMicroSeconds()
      << " " << it->RealForwardDeltaOWD[1].GetMicroSeconds()
      << " " << it->RealReverseDeltaOWD[1].GetMicroSeconds()
      << " " << it->EstimatedForwardDeltaOWD[0].GetMicroSeconds()
      << " " << it->EstimatedReverseDeltaOWD[0].GetMicroSeconds()
      << " " << it->EstimatedForwardDeltaOWD[1].GetMicroSeconds()
      << " " << it->EstimatedReverseDeltaOWD[1].GetMicroSeconds()
      << " " << (it->rtt[0].GetMicroSeconds()/2)
      << " " << (it->rtt[1].GetMicroSeconds()/2)
      << " " << it->FastestForwardSubflow << std::endl;
  }

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
