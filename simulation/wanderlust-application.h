/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * Copyright 2013 Gerard Krol
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

#ifndef WANSERLUST_APPLICATION_H
#define WANSERLUST_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include <vector>
#include <map>
#include "wanderlustpeer.h"

namespace ns3 {

class Socket;
class Packet;

class Wanderlust : public Application 
{
public:
  static TypeId GetTypeId (void);
  Wanderlust ();
  virtual ~Wanderlust ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void HandleRead (Ptr<Socket> socket);
  void SendSwapRequest(void);
  void SendHello(void);

  std::vector< Ptr<Socket> > sockets;
  Address m_local;
  EventId m_sendSwapRequestEvent;
  EventId m_sendHelloEvent;

  std::map<pubkey_t, WanderlustPeer> peers;
  pubkey_t pubkey;
  location_t location;
};

} // namespace ns3

#endif /* WANSERLUST_APPLICATION_H */

