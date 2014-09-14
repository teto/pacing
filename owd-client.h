
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

#include "config.h"

using namespace ns3;



typedef struct _rttSample
{
Time real;      //!< computed thanks to ns3 clock synchronization
Time estimated; //!< estimated via our technique
Time halfRTT;   //!< legacy method of dividing RTT by half
} RttSample;

typedef struct _roundStats
{
RttSample samples[10];
} RoundStats;


//std::vector<fastestSocket>

/**
 * \ingroup OWDHostserver
 * \class OWDHost
 * \brief A Udp client. Sends UDP packet carrying sequence number and time stamp
 *  in their payloads

 It should change mode automatically
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
  We suppose there is no packet loss
  **/
  void SamplingRTTRecv(int sockId, const SeqTsHeader& seqTs);
  void EstimateOWDRecv(int sockId, const SeqTsHeader& seqTs);

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


  /**
  Will schedule different sends on different subflows in order to compute
  **/
  void EstimateOWDStart ( );

  /** Utility function used by both modes.
  Send a timestampped packet with "seqNb" on socket "sock"
  \param seqNb sequence number to send
  **/
  void Send(Ptr<Socket> sock, uint32_t seqNb);

  uint32_t m_count; //!< Maximum number of packets the application will send
//  TcpTxBuffer m_txBuffer; //!< Just used to see what packets.

  //! How many packets we can send in parallel
  uint32_t m_probesInARound;

  //! Sampled RTTs
  std::vector<RttSample> m_RttSamples[10]; //!< one tracedvalue per socket (supports max 10 sockets)


  Mode m_currentMode; //!< decide what actions to take

  uint32_t m_sampleRTTmaxRounds;  //!< How many times we sample the RTT before changing mode
  uint32_t m_round;   //!< nb of rounds already accomplished


  // maybe this should go away
//  Time m_interval; //!< Packet inter-send time

  uint32_t m_size; //!< Size of the sent packet (including the SeqTsHeader)

  uint32_t m_inflight; //!< Nb of packets in flight. Starts at 0
  std::vector<Ptr<Socket> > m_sockets;
  Ptr<Node> m_peer;

  EventId m_sendEvent; //!< Event to send the next packet


  RoundStats m_currentRoundStats; //!<
//  std::vector<Time> m_rttBuffer;  //!<
  std::vector< std::pair<int,int> > m_forwardOrder; //!< socket no/position registered by packets of nb(sockets) records
};


#endif
