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

class SwapRoutingDestination {
public:
    SwapRoutingDestination() {
        flowid = 0;
    }
    bool operator<(const SwapRoutingDestination &other) const {
        if (flowid < other.flowid) return true;
        if (flowid > other.flowid) return true;
        return pubkey < other.pubkey;
    }
    Pubkey pubkey;
    uint32_t flowid;
};

class SwapRoutingNextHop {
public:
    SwapRoutingNextHop() : gateway(NULL), created(0) {
    }
    SwapRoutingNextHop(WanderlustPeer *gateway) : gateway(gateway), created(0) {
    }
    WanderlustPeer *gateway;
    uint64_t created;
};

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
    double calculateDistance(Location &location1, Location &location2);
    double calculateLocationError(Location &location);
    bool shouldSwapWith(Pubkey &peer_pubkey, Location &peer_location);

    void processSwapRequest(WanderlustPeer &peer, WanderlustHeader& header);
    void processSwapResponse(WanderlustPeer &peer, WanderlustHeader& header);
    void processSwapConfirmation(WanderlustPeer &peer, WanderlustHeader& header);

    std::vector< Ptr<Socket> > sockets;
    Address m_local;
    EventId m_sendSwapRequestEvent;
    EventId m_sendHelloEvent;

    std::map<Pubkey, WanderlustPeer> peers;
    std::map<Ptr<Socket>,WanderlustPeer*> socketToPeer;
    Pubkey pubkey;
    Location location;
    bool swapInProgress;
    double swapTimeOut;

    std::map<SwapRoutingDestination, SwapRoutingNextHop> swapRoutingTable;
};

} // namespace ns3

#endif /* WANSERLUST_APPLICATION_H */

