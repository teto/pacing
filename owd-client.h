
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

#include "config.h"

using namespace ns3;

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

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void Send (void);

  uint32_t m_count; //!< Maximum number of packets the application will send

  // maybe this should go away
  Time m_interval; //!< Packet inter-send time
  uint32_t m_size; //!< Size of the sent packet (including the SeqTsHeader)

  uint32_t m_sent; //!< Counter for sent packets
  std::vector<Ptr<Socket> > m_sockets;
  Ptr<Node> m_peer;
//  [NUM_INTERFACES];      //!< Socket
//  Ipv4Address m_peerAddresses[NUM_INTERFACES]; //!< Remote peer address
//  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet

};


#endif /* UDP_CLIENT_H */
