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

#define UINT64_MAX (18446744073709551615ULL)

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
class WanderlustHeader;

class Wanderlust : public Application 
{
public:
    static TypeId GetTypeId (void);
    Wanderlust ();
    virtual ~Wanderlust ();
    double getLocation() {
      return *(uint64_t*)location.data/(double)UINT64_MAX;
    }
    double getLocationError() {
      return calculateLocationError(location);
    }
protected:
    virtual void DoDispose (void);

private:

    virtual void StartApplication (void);
    virtual void StopApplication (void);

    void HandleRead (Ptr<Socket> socket);
    void SendSwapRequest(void);
    void SendHello(void);

    /// Lower is better
    double calculateDistance(location_t &location1, location_t &location2);
    double calculateLocationError(location_t &location);
    bool shouldSwapWith(pubkey_t &peer_pubkey, location_t &peer_location);

    void processSwapRequest(const Ptr<Socket>& socket, WanderlustHeader& header);
    void processSwapResponse(const Ptr<Socket>& socket, WanderlustHeader& header);
    void processSwapConfirmation(WanderlustHeader& header);
    void processHello(const WanderlustHeader& header,
            const Ptr<Socket>& socket);

    std::vector< Ptr<Socket> > sockets;
    Address m_local;
    EventId m_sendSwapRequestEvent;
    EventId m_sendHelloEvent;

    std::map<pubkey_t, WanderlustPeer> peers;
    pubkey_t pubkey;
    location_t location;
    bool swapInProgress;
    double swapTimeOut;
};

} // namespace ns3

#endif /* WANSERLUST_APPLICATION_H */

