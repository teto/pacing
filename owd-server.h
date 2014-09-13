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

#ifndef UDP_OWD_ECHO_SERVER_H
#define UDP_OWD_ECHO_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "ns3/packet.h"


using namespace ns3;

/**
 *
 * Every packet received is sent back.
 * it
 */
class OwdServer : public ns3::Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
//  static TypeId GetTypeId (void);
  OwdServer ();
  virtual ~OwdServer ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (ns3::Ptr<ns3::Socket> socket);

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  std::vector< Ptr<Socket> > m_sockets; //!< IPv4 Socket

  // TODO create structure that records seq_nb.

//  Ptr<Socket> m_socket6; //!< IPv6 Socket
//  Address m_local; //!< local multicast address
};


#endif /* UDP_ECHO_SERVER_H */

