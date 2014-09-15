
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
 * Author:
 *
 */

#ifndef UDP_OWD_CLIENT_H
#define UDP_OWD_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/tcp-tx-buffer.h"
#include "ns3/traced-value.h"
#include "ns3/seq-ts-header.h"

#include "config.h"
#include "sequencer.h"

using namespace ns3;



//typedef struct _rttSample
//{
//Time real;      //!< computed thanks to ns3 clock synchronization
//Time estimated; //!< estimated via our technique
//Time halfRTT;   //!< legacy method of dividing RTT by half
//} RttSample;

//class RttRoundStats
//{
//  RttRoundStats();
//  Time rtt[10];
//};

/**
Only FastestForwardSubflow and rtt are used in RTTsampling mode
**/
class RoundStats
{
public:
  RoundStats();
  Time EstimatedForwardDeltaOWD;
  Time RealForwardDeltaOWD[10];
  Time EstimatedReverseDeltaOWD;
  Time RealReverseDeltaOWD[10];
  Time rtt[10];
  Time EstimatedForwardDelay[10];
  Time EstimatedReverseDelay[10];
  int ForwardFastSubflow;  //!< Used
//RttSample samples[10];
};

typedef std::vector<RoundStats> RoundStatsCollection;

//std::vector<fastestSocket>

/**
OWDHost first runs a few RTT samplings to guess which subflow is the first to arrive at the receiver.
When enough samples have been gathered, it switches to ForwardDeltaOWD estimation mode in which it delays packets
on fast path in order to guess DEltaOWD.
Once enough DeltaOWD estimations have been done, then program closes

Ideally it should be able to switch back and forth between RTT and OWD sampling in case delays change much o

 *
 */
class OWDHost : public Application
{
public:

  OWDHost ();

  virtual ~OWDHost ();

//  void SetRemote ( uint16_t id, Ipv4);
  void
  HandleRecv( Ptr<Socket> packet );

  void
  SetPeer( Ptr<Node> peer )
  {
    m_peer = peer;
  }

  /**
  Returns the delay to wait after a packet on slow path
  before sending the "i" th probe
  */
  Time GetDelayOfProbe(uint8_t i) const;

  /**
  We suppose there is no packet loss
  **/
  void SamplingRTTRecv(int sockId, const SeqTsHeader& seqTs);
  void EstimateOWDRecv(int sockId, const SeqTsHeader& seqTs);

  /** Outputs records in os */
  void DumpRttSamples(std::ostream& os) const;
  void DumpOwdSamples(std::ostream& os) const;

  int ForwardFastSubflowId() const;
  int ForwardSlowSubflowId() const;

protected:
  virtual void DoDispose (void);

private:
  void ChangeMode(Mode mode);

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /** To sample RTTs, we just send timestampped packets in parallel on each *subflow*
  When they are echoed back we register the RTT in a tracedvalue

  The seq nb allows to distinguish which packets arrived first
  The seq nb are sent increasing according to the sorting order of the sockets, ie:
  seq will be sent on socket 0 and seq+1 on socket 1

  **/
  void SampleRTTStart (void);

  /** Socket must exist (else asserts)
  **/
  int GetIndexOfSocket(Ptr<Socket> sock);

  /** Depending on mode, returns number of recorded samples
  */
  int GetRoundNo() const;

  /**
  Will try to estimate delta forward one way delay based on up
  (smoothed ?) 3 past measurements
  */
//  Time EstimateForwardOWDs(fOWD1,fOWD2);

  /**
  Will schedule different sends on different subflows in order to compute
  **/
  void EstimateOWDStartNewRound ( );

  /** Utility function used by both modes.
  Send a timestampped packet with "seqNb" on socket "sock"
  \param seqNb sequence number to send
  **/
  void Send(Ptr<Socket> sock, uint32_t seqNb);

//  uint32_t m_count; //!< Maximum number of packets the application will send
//  TcpTxBuffer m_txBuffer; //!< Just used to see what packets.

  //! How many packets we can send in parallel
  const uint32_t m_probesInARound;  //!< Number of probes in a round (default 3);
  const uint32_t m_sampleRTTmaxRounds;  //!< How many times we sample the RTT before changing mode
  const uint32_t m_owdMaxRounds;  //!< How many times we sample the RTT before changing mode

  //! Sampled RTTs
//  std::vector<RttSample> m_RttSamples[10]; //!< one tracedvalue per socket (supports max 10 sockets)


  Mode m_currentMode; //!< decide what actions to take


//  uint32_t m_round;   //!< nb of rounds already accomplished


  // maybe this should go away
//  Time m_interval; //!< Packet inter-send time

  uint32_t m_highestAcknowledgedAckInRound;
  uint32_t m_inflight; //!< Nb of packets in flight. Starts at 0
  std::vector<Ptr<Socket> > m_sockets;
  Ptr<Node> m_peer;

  EventId m_sendEvent; //!< Event to send the next packet

  RoundStatsCollection m_owdRoundStats;
  RoundStatsCollection m_rttRoundStats;
  RoundStats m_currentRoundStats; //!<
//  std::vector<Time> m_rttBuffer;  //!<
//  std::vector< std::pair<int,int> > m_forwardOrder; //!< socket no/position registered by packets of nb(sockets) records


  //// Variables used in OWD mode
  ////////////////////////////////////////////////////////
  int m_forwardFastSubflow; //!<
  Time m_estimatedForwardDeltaOwd;

  /** arrival position at the server side, not on the client sie
  so it means order is deduced from seq nb
  */
  int m_arrivalPositionSlowPacket;  //!<
  int m_arrivalPositionLastProbeBeforeSlowPath;  //!<
  int m_arrivalPositionFirstProbeAfterSlowPath;  //!<

  // Can be deduced from precedent variable
//  Time m_timeOfLastProbeBeforeSlowPath; //!< Not necessarily set
//  Time m_timeOfFirstProbeAfterSlowPath; //!< Not necessarily set
//  Sequencer m_sequencer;  //!< No need ?
};


#endif
