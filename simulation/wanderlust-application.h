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

using namespace std;

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
    double getLocation2() {
      return ((uint64_t*)location.data)[1]/(double)UINT64_MAX;
    }
    string getLocationText() {
        ostringstream stream;
        stream << getLocation() << "\\n" << getLocation2();
        return stream.str();
    }
    double getLocationError() {
      return calculateLocationError(location);
    }
    void setPosition(double x, double y) {
        this->x = x;
        this->y = y;
    }
    double calculateDistanceSquared(Wanderlust &other) {
        return std::pow(x-other.x,2)+std::pow(y-other.y,2);
    }
    double getX() { return x; }
    double getY() { return y; }
    uint16_t getShortId() {
        return pubkey.getShortId();
    }
    uint32_t getSentPingCount() {
        return sentPingCount;
    }
    uint32_t getReceivedPongCount() {
        return receivedPongCount;
    }
    void resetStats() {
        sentPingCount = 0;
        receivedPongCount = 0;
    }
protected:
    virtual void DoDispose (void);

private:

    virtual void StartApplication (void);
    virtual void StopApplication (void);

    void HandleRead (Ptr<Socket> socket);
    void SendSwapRequest(void);
    void SendScheduledHello(void);
    void SendPing(void);

    /// Lower is better
    double calculateDistance(Location &location1, Location &location2);
    double calculateLocationError(Location &location);
    bool shouldSwapWith(Pubkey &peer_pubkey, Location &peer_location);

    void processSwapRequest(WanderlustPeer &peer, WanderlustHeader& header);
    void processSwapResponse(WanderlustPeer &peer, WanderlustHeader& header);
    void processSwapConfirmation(WanderlustPeer &peer, WanderlustHeader& header);
    void processSwapRefusal(WanderlustPeer &peer, WanderlustHeader& header);
    void processPing(WanderlustPeer &peer, WanderlustHeader& header);
    void processPong(WanderlustPeer &peer, WanderlustHeader& header);
    void SendHello();

    void snoopLocation(WanderlustHeader& header);

    void route(Ptr<Packet> packet, WanderlustHeader& header);

    std::vector< Ptr<Socket> > sockets;
    Address m_local;
    EventId m_sendSwapRequestEvent;
    EventId m_sendHelloEvent;
    EventId m_sendPingEvent;

    std::map<Pubkey, WanderlustPeer> peers;
    std::map<Ptr<Socket>,WanderlustPeer*> socketToPeer;
    Pubkey pubkey;
    Location location;
    bool swapInProgress;
    double swapTimeOut;
    Pubkey swapPartner;

    std::map<SwapRoutingDestination, SwapRoutingNextHop> swapRoutingTable;
    double x,y;
    static const double swapTimeOutTime = 60;

    map<Pubkey, Location> locationStore;
    uint32_t sentPingCount;
    uint32_t receivedPongCount;
};

} // namespace ns3

#endif /* WANSERLUST_APPLICATION_H */

