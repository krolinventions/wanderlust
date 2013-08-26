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

#include <cmath>
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/channel.h"
#include "ns3/ipv4.h"

#include "wanderlust-application.h"
#include "wanderlust-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WanderlustApplication");
NS_OBJECT_ENSURE_REGISTERED (Wanderlust);

TypeId
Wanderlust::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Wanderlust")
    .SetParent<Application> ()
    .AddConstructor<Wanderlust> ();
  return tid;
}

static void fillWithRandomData(uint8_t *buffer, size_t size) {
    for (size_t i=0;i< size;i++)
        buffer[i] = rand()%256;
}

Wanderlust::Wanderlust ()
{
  NS_LOG_FUNCTION (this);
  fillWithRandomData(pubkey.data, sizeof(pubkey));
  fillWithRandomData(location.data, sizeof(location));
  swapInProgress = false;
  swapTimeOut = Simulator::Now().GetSeconds();
  sentPingCount = 0;
  receivedPongCount = 0;
}

Wanderlust::~Wanderlust()
{
  NS_LOG_FUNCTION (this);
}

void
Wanderlust::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
Wanderlust::StartApplication (void)
{
    NS_LOG_FUNCTION (this);
    pubkey.setShortId(1000+GetNode()->GetId()); // can't do this in the constructor

    // We enumerate all our network devices and bind a socket to every one of them
    for (unsigned int i=0;i<GetNode()->GetNDevices();i++) {
        Ptr<NetDevice> device = GetNode()->GetDevice(i);
        if (!device->GetChannel()) continue; // it's not connected
        NS_LOG_INFO ( "Found network device " << device->GetAddress() << " channel " << device->GetChannel());
        
        Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress(ipv4->GetInterfaceForDevice(device),0);
        
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        Ptr<Socket> socket = Socket::CreateSocket (GetNode (), tid);
        InetSocketAddress local = InetSocketAddress (iaddr.GetLocal(), 6556);
        socket->Bind(local);
        socket->BindToNetDevice(device);
        //NS_LOG_INFO ( "Binding to " << InetSocketAddress::ConvertFrom (local).GetIpv4 () << " result " << result << " bound to " << socket->GetBoundNetDevice());
        socket->SetAllowBroadcast(true);
        socket->SetRecvCallback (MakeCallback (&Wanderlust::HandleRead, this));
        sockets.push_back(socket);
    }

    m_sendSwapRequestEvent = Simulator::Schedule(Seconds (rand()%50/10.0+10), &Wanderlust::SendSwapRequest, this);
    m_sendHelloEvent = Simulator::Schedule(Seconds (rand()%50/10.0), &Wanderlust::SendScheduledHello, this);
    m_sendPingEvent = Simulator::Schedule(Seconds (rand()%100/10.0+5), &Wanderlust::SendPing, this);
    SendHello();
}

void 
Wanderlust::StopApplication ()
{
    NS_LOG_FUNCTION (this);
    
    for (std::vector< Ptr<Socket> >::iterator it=sockets.begin();it!=sockets.end();++it) {
        Ptr<Socket> socket = *it;
        socket->Close ();
        socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
    Simulator::Cancel (m_sendSwapRequestEvent);
    Simulator::Cancel (m_sendHelloEvent);
    Simulator::Cancel (m_sendPingEvent);
}

void Wanderlust::processSwapRequest(WanderlustPeer &peer, WanderlustHeader& header) {
    // record in routing table
    SwapRoutingDestination destination;
    destination.pubkey = header.contents.src_pubkey;
    SwapRoutingNextHop nextHop(&peer);
    swapRoutingTable[destination] = nextHop;

    if (rand()%100 < 95) { // FIXME: replace by hop limit?
        // let's forward it!
        unsigned int choice = rand()%peers.size();
        for (std::map<Pubkey,WanderlustPeer>::iterator it=peers.begin();it!=peers.end();++it) {
            if (choice > 0) {
                choice--;
                continue;
            }
            // we've picked a peer
            Ptr<Packet> forwardedPacket = Create<Packet>();
            forwardedPacket->RemoveAllByteTags();
            forwardedPacket->RemoveAllPacketTags();
            forwardedPacket->AddHeader(header);
            NS_LOG_INFO("Forwarding swap request " << header);
            it->second.socket->SendTo(forwardedPacket, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 6556));
            break;
        }

        return;
    }

    if (swapInProgress) {
        NS_LOG_INFO("Ignoring swap request (in progress)");
        return;
    }

    // check if we would improve
    uint8_t swapBits = header.contents.flow_id1;
    if (shouldSwapWith(header.contents.src_pubkey, header.contents.src_location, swapBits, true)) {
        NS_LOG_DEBUG("Detected potentially advantageous swap, responding " << (int)swapBits);
        Ptr<Packet> swapResponsePacket = Create<Packet>();
        WanderlustHeader swapResponseHeader;
        swapResponseHeader.contents.src_location = location;
        swapResponseHeader.contents.src_pubkey = pubkey;
        swapResponseHeader.contents.dst_location = header.contents.src_location;
        swapResponseHeader.contents.dst_pubkey = header.contents.src_pubkey;
        swapResponseHeader.contents.message_type = WANDERLUST_TYPE_SWAP_RESPONSE;
        swapResponseHeader.contents.flow_id1 = swapBits;
        swapResponsePacket->AddHeader(swapResponseHeader);
        peer.socket->SendTo(swapResponsePacket, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 6556));

        swapInProgress = true;
        swapTimeOut = Simulator::Now().GetSeconds() + swapTimeOutTime;
        swapPartner = header.contents.src_pubkey;
    }
}

void Wanderlust::processSwapResponse(WanderlustPeer &peer, WanderlustHeader& header) {
    // we've got a response, check if it would be a good idea to swap
    uint8_t swapBits = header.contents.flow_id1;
    if (header.contents.dst_location != location) {
        NS_LOG_DEBUG("SwapResponse: Our location has changed in the meantime, discarding");
        goto send_refusal;
    }
    if (swapInProgress) {
        NS_LOG_DEBUG("Swap already in progress");
        goto send_refusal;
    }
    if (shouldSwapWith(header.contents.src_pubkey, header.contents.src_location, swapBits, false)) {
        NS_LOG_DEBUG("SWAPPING on response " << (int)swapBits);
        Ptr<Packet> swapResponsePacket = Create<Packet>();
        WanderlustHeader swapResponseHeader;
        swapResponseHeader.contents.src_location = location;
        swapResponseHeader.contents.src_pubkey = pubkey;
        swapResponseHeader.contents.dst_location = header.contents.src_location;
        swapResponseHeader.contents.dst_pubkey = header.contents.src_pubkey;
        swapResponseHeader.contents.message_type = WANDERLUST_TYPE_SWAP_CONFIRMATION;
        swapResponseHeader.contents.flow_id1 = swapBits;
        swapResponsePacket->AddHeader(swapResponseHeader);
        peer.socket->SendTo(swapResponsePacket, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 6556));
        // FIXME: keep sending until we receive a confirmation

        // perform the swap
        swapByBits(location, header.contents.src_location, swapBits);
        SendHello();
        swapInProgress = true;
        swapTimeOut = Simulator::Now().GetSeconds() + swapTimeOutTime;
        swapPartner = header.contents.src_pubkey;
        return;
    } else {
        NS_LOG_DEBUG("swap won't lower our location error");
    }

send_refusal:
    Ptr<Packet> swapResponsePacket = Create<Packet>();
    WanderlustHeader swapResponseHeader;
    swapResponseHeader.contents.src_location = location;
    swapResponseHeader.contents.src_pubkey = pubkey;
    swapResponseHeader.contents.dst_location = header.contents.src_location;
    swapResponseHeader.contents.dst_pubkey = header.contents.src_pubkey;
    swapResponseHeader.contents.message_type = WANDERLUST_TYPE_SWAP_REFUSAL;
    swapResponsePacket->AddHeader(swapResponseHeader);
    peer.socket->SendTo(swapResponsePacket, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 6556));
}

void Wanderlust::processSwapConfirmation(WanderlustPeer &peer, WanderlustHeader& header) {
    if (!swapInProgress) {
        NS_LOG_WARN("Did not expect swap confirmation, no swap in progress");
        return; // we're not swapping, why did we get this packet?
    }
    if (header.contents.src_pubkey != swapPartner) {
        NS_LOG_WARN("This is not the node we're currently swapping with");
        return;
    }

    swapInProgress = false;

    // we've got a response, check if it would be a good idea to swap
    if (header.contents.dst_location != location) {
        NS_LOG_WARN("Our location has changed in the meantime, discarding");
        return;
    }
    uint8_t swapBits = header.contents.flow_id1;
    if (!shouldSwapWith(header.contents.src_pubkey, header.contents.src_location, swapBits, false)) {
        // bad luck, we should go ahead anyway
        NS_LOG_DEBUG("Confirmed swap won't lower our location error, still going ahead");
    }
    NS_LOG_DEBUG("SWAPPING on confirmation " << (int)swapBits);
    // perform the swap
    swapByBits(location, header.contents.src_location, swapBits);
    SendHello();
}

void Wanderlust::processSwapRefusal(WanderlustPeer &peer, WanderlustHeader& header) {
    if (!swapInProgress) {
        NS_LOG_WARN("did not expect swap refusal, no swap in progress");
        return; // we're not swapping, why did we get this packet?
    }
    if (header.contents.src_pubkey != swapPartner) {
        NS_LOG_WARN("this is not the node we're currently swapping with");
        return;
    }
    NS_LOG_DEBUG("ok, swap refused");
    swapInProgress = false;
}

void 
Wanderlust::HandleRead (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    socket->GetBoundNetDevice()->GetIfIndex();

    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom (from)))
    {
        NS_LOG_INFO ("Received " << packet->GetSize () << " bytes from " <<
            InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
            InetSocketAddress::ConvertFrom (from).GetPort ());
        WanderlustHeader header;
        packet->PeekHeader(header);
        NS_LOG_INFO (header);

        // snoop for pubkey->location mappings
        snoopLocation(header);

        // update socket-peer mapping
        if (header.contents.message_type == WANDERLUST_TYPE_HELLO) {
            if (peers.count(header.contents.src_pubkey) == 0) {
                // we don't have it yet, create a new one
                peers[header.contents.src_pubkey] = WanderlustPeer();
            }
            // update the location
            peers[header.contents.src_pubkey].location = header.contents.src_location;
            peers[header.contents.src_pubkey].pubkey = header.contents.src_pubkey;
            peers[header.contents.src_pubkey].socket = socket;
            socketToPeer[socket] = &peers[header.contents.src_pubkey];
            // log our current error
            NS_LOG_INFO("current error " << calculateLocationError(location));
            return; // done here
        }
        WanderlustPeer &peer = *socketToPeer[socket]; // FIXME: ugly

        if (header.contents.message_type == WANDERLUST_TYPE_SWAP_REQUEST) {
            processSwapRequest(peer, header);
            return;
        }

        if (header.contents.message_type == WANDERLUST_TYPE_SWAP_RESPONSE ||
            header.contents.message_type == WANDERLUST_TYPE_SWAP_CONFIRMATION ||
            header.contents.message_type == WANDERLUST_TYPE_SWAP_REFUSAL) {
            // add to the routing table just to be sure
            SwapRoutingDestination srcDestination;
            srcDestination.pubkey = header.contents.src_pubkey;
            SwapRoutingNextHop nextHop(&peer);
            swapRoutingTable[srcDestination] = nextHop;

            if (header.contents.dst_pubkey == pubkey) {
                switch (header.contents.message_type) {
                    case WANDERLUST_TYPE_SWAP_RESPONSE:
                        processSwapResponse(peer, header);
                        break;
                    case WANDERLUST_TYPE_SWAP_CONFIRMATION:
                        processSwapConfirmation(peer, header);
                        break;
                    case WANDERLUST_TYPE_SWAP_REFUSAL:
                        processSwapRefusal(peer, header);
                        break;
                    default:
                        NS_LOG_WARN("Unknown message type " << header.contents.message_type);
                        break;
                }
            } else {
                // these swap messages are not for us, let's forward
                SwapRoutingDestination destination;
                destination.pubkey = header.contents.dst_pubkey;
                if (swapRoutingTable.count(destination)) {
                    packet->RemoveAllByteTags();
                    packet->RemoveAllPacketTags();
                    swapRoutingTable[destination].gateway->socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 6556));
                    NS_LOG_INFO ("FORWARDING " << header);
                } else {
                    NS_LOG_WARN ("can't forward " << header << " no entry in routing table");
                }
            }
            return;
        }
        if (header.contents.dst_pubkey != pubkey) {
            packet->RemoveAllByteTags();
            packet->RemoveAllPacketTags();
            route(packet, header);
            return;
        }
        switch (header.contents.message_type) {
            case WANDERLUST_TYPE_DATA: break;
            case WANDERLUST_TYPE_LOCATION_QUERY: break;
            case WANDERLUST_TYPE_LOCATION_ANSWER: break;
            case WANDERLUST_TYPE_HELLO:
                // already handled
                break;
            case WANDERLUST_TYPE_PING:
                processPing(peer, header);
                break;
            case WANDERLUST_TYPE_PONG:
                processPong(peer, header);
                break;
            default:
                NS_LOG_WARN("Unknown message type " << header.contents.message_type);
                break;
        }
    }
}

void Wanderlust::SendSwapRequest(void) {
    NS_LOG_FUNCTION(this);
    
    if (peers.size()) {
        int target = rand()%peers.size();
        for (std::map<Pubkey,WanderlustPeer>::iterator it=peers.begin();it!=peers.end();++it) {
            if (target > 0) {
                target--;
                continue;
            }

            WanderlustPeer &peer = it->second;

            Ptr<Packet> p = Create<Packet>();

            WanderlustHeader header;
            header.contents.message_type = WANDERLUST_TYPE_SWAP_REQUEST;
            header.contents.src_pubkey = pubkey;
            header.contents.src_location = location;
            header.contents.hop_limit = 1;
            header.contents.flow_id1 = 0xff;
            p->AddHeader(header);

            NS_LOG_INFO ("Sending " << header);
            peer.socket->SendTo(p,0,InetSocketAddress (Ipv4Address::GetBroadcast(), 6556));
            break;
        }
    }
    // do this afterwards so we have some time in which to receive those messages
    if (swapInProgress && swapTimeOut < Simulator::Now().GetSeconds()) {
        NS_LOG_WARN("Swap timeout!");
        swapInProgress = false;
    }

    m_sendSwapRequestEvent = Simulator::Schedule(Seconds(1+rand()%100/100.0), &Wanderlust::SendSwapRequest, this);
}

void Wanderlust::SendHello() {
    for (std::vector<Ptr<Socket> >::iterator it = sockets.begin();
            it != sockets.end(); ++it) {
        Ptr<Socket> socket = *it;
        NS_LOG_INFO ("Sending hello packet");
        Ptr<Packet> p = Create<Packet>();
        //m_txTrace (p); ??
        WanderlustHeader header;
        header.contents.message_type = WANDERLUST_TYPE_HELLO;
        header.contents.src_pubkey = pubkey;
        header.contents.src_location = location;
        p->AddHeader(header);
        socket->SendTo(p, 0,
                InetSocketAddress(Ipv4Address::GetBroadcast(), 6556));
    }
}

void Wanderlust::SendScheduledHello(void) {
    NS_LOG_FUNCTION(this);

    if (!swapInProgress) {
        // perturb the location a bit to keep things going and to increase network health
        // what would be reasonable? -> changing entire location in 1 hour
        //((int32_t*)location.data)[1] += rand()%0x02000000 - 0x01000000;
        //((int32_t*)location.data)[3] += rand()%0x02000000 - 0x01000000;
    }

    SendHello();
    m_sendHelloEvent = Simulator::Schedule(Seconds (60 + (rand()%6000)/100.0), &Wanderlust::SendScheduledHello, this);
}

double Wanderlust::calculateDistance(Location &location1, Location &location2) {
    if (dimensions == 0) return 0;
    if (dimensions == 1 || dimensions == 2) {
        double error1a = std::abs(((uint64_t*)location1.data)[0]/(double)UINT64_MAX - ((uint64_t*)location2.data)[0]/(double)UINT64_MAX);
        double error2a = std::abs(std::abs(((uint64_t*)location1.data)[0]/(double)UINT64_MAX - ((uint64_t*)location2.data)[0]/(double)UINT64_MAX) - 1);
        if (dimensions == 2) {
            double error1b = std::abs(((uint64_t*)location1.data)[1]/(double)UINT64_MAX - ((uint64_t*)location2.data)[1]/(double)UINT64_MAX);
            double error2b = std::abs(std::abs(((uint64_t*)location1.data)[1]/(double)UINT64_MAX - ((uint64_t*)location2.data)[1]/(double)UINT64_MAX) - 1);
            return std::pow(std::pow(std::min(error1a,error2a),2)+std::pow(std::min(error1b,error2b),2), 0.25);
        }
        return std::sqrt(std::min(error1a,error2a));
    }
    if (dimensions == -1) {
        // 1D in only one direction
        double error = ((uint64_t*)location1.data)[0]/(double)UINT64_MAX - ((uint64_t*)location2.data)[0]/(double)UINT64_MAX;
        if (error < 0) return 1;
        return error;
    }
    if (dimensions == -10) {
        // 1D linear
        return std::abs(((uint64_t*)location1.data)[0]/(double)UINT64_MAX - ((uint64_t*)location2.data)[0]/(double)UINT64_MAX);
    }
    if (dimensions == -20) {
        // 2D linear
        double error1a = std::abs(((uint64_t*)location1.data)[0]/(double)UINT64_MAX - ((uint64_t*)location2.data)[0]/(double)UINT64_MAX);
        double error1b = std::abs(((uint64_t*)location1.data)[1]/(double)UINT64_MAX - ((uint64_t*)location2.data)[1]/(double)UINT64_MAX);
        return std::pow(std::pow(error1a,2)+std::pow(error1b,2), 0.25);
    }
    if (dimensions >= 3 && dimensions <=4 ) {
        double acc = 0;
        for (int i=0;i<dimensions;i++) {
            double error1 = std::abs(((uint32_t*)location1.data)[i]/(double)UINT32_MAX - ((uint32_t*)location2.data)[i]/(double)UINT32_MAX);
            double error2 = std::abs(std::abs(((uint32_t*)location1.data)[i]/(double)UINT32_MAX - ((uint32_t*)location2.data)[i]/(double)UINT32_MAX) - 1);
            acc += std::pow(std::min(error1,error2),2);
        }
        return std::pow(acc, 0.25);
    }
    if (dimensions >= 5 && dimensions <= 8) {
        double acc = 0;
        for (int i=0;i<dimensions;i++) {
            double error1 = std::abs(((uint16_t*)location1.data)[i]/(double)0xffff - ((uint16_t*)location2.data)[i]/(double)0xffff);
            double error2 = std::abs(std::abs(((uint16_t*)location1.data)[i]/(double)0xffff - ((uint16_t*)location2.data)[i]/(double)0xffff) - 1);
            acc += std::pow(std::min(error1,error2),2);
        }
        return std::pow(acc, 0.25);
    }
    if (dimensions == 64 || dimensions == 128) {
        uint64_t changedBits = ((uint64_t*)location1.data)[0] ^ ((uint64_t*)location2.data)[0];
        size_t count = 0;

        while (changedBits) {
            if (changedBits & 1) count++;
            changedBits >>= 1;
        }
        if (dimensions == 64)
            return count/64.0;

        changedBits = ((uint64_t*)location1.data)[1] ^ ((uint64_t*)location2.data)[1];
        while (changedBits) {
            if (changedBits & 1) count++;
            changedBits >>= 1;
        }
        return count/128.0;
    }
    if (dimensions == -64) {
        // kademlia
        return std::log((((uint64_t*)location1.data)[0] ^ ((uint64_t*)location2.data)[0]))/std::log(UINT64_MAX);
    }
    cerr << "Unsupported number of dimensions " << dimensions;
    exit(1); // bye!
}

double Wanderlust::calculateLocationError(Location &location) {
    double error = 0;
    for (std::map<Pubkey,WanderlustPeer>::iterator it=peers.begin();it!=peers.end();++it) {
        error += calculateDistance(location, it->second.location)/peers.size();
    }
    return error;
}

bool Wanderlust::shouldSwapWith(Pubkey &peer_pubkey, Location &peer_location, uint8_t &swapBits, bool setSwapBits) {
    if (!swap) return false; // swapping is disabled

    double currentError = calculateLocationError(location);
    if (!independentSwap) {
        double newError = calculateNewError(peer_pubkey, peer_location, location);
        NS_LOG_INFO ("old error " << currentError << " new error " << newError);
        if (setSwapBits) swapBits = 0xff;
        return newError <= currentError; // swap if the same error, makes sure swapping in long chains works
    } else {
        if (setSwapBits) {
            // now the fun begins
            // we need to determine which parts to swap in order to get the biggest location improvement
            // we set the bits in swapBits and return true if it is an improvement
            uint8_t allowedSwapBits = swapBits;
            if (allowedSwapBits == 0) allowedSwapBits = 0xff; // everything goes!

            uint8_t bestSwapBits = 0;
            double bestError = 0;
            unsigned int max = 1 << dimensions;
            for (uint8_t s=1;s<max;s++) {
                uint8_t targetBits = allowedSwapBits & s;
                if (!targetBits) continue;
                Location my   = location;
                Location peer = peer_location;
                swapByBits(my, peer, targetBits);
                double newError = calculateNewError(peer_pubkey, my, peer);
                if (bestSwapBits == 0 || newError < bestError) {
                    bestSwapBits = targetBits;
                    bestError = newError;
                }
            }
            if (!bestSwapBits) return false;
            swapBits = bestSwapBits;
            return bestError <= currentError;
        } else {
            // just check the swap
            Location my   = location;
            Location peer = peer_location;
            swapByBits(my, peer, swapBits);
            double newError = calculateNewError(peer_pubkey, my, peer);
            return newError <= currentError;
        }
    }
}

void Wanderlust::swapByBits(Location &a, Location &b, uint8_t swapBits)
{
    if (dimensions == 1) {
        Location tmp = a;
        a = b;
        b = tmp;
        return;
    }
    if (dimensions == 2) {
        for (int i=0;i<2;i++) {
            if (swapBits&(1<<i)) {
                // swap part 1
                ((uint64_t*)a.data)[i] ^= ((uint64_t*)b.data)[i];
                ((uint64_t*)b.data)[i] ^= ((uint64_t*)a.data)[i];
                ((uint64_t*)a.data)[i] ^= ((uint64_t*)b.data)[i];
            }
        }
        return;
    }
    if (dimensions <= 4) {
        for (int i=0;i<dimensions;i++) {
            if (swapBits&(1<<i)) {
                // swap part 1
                ((uint32_t*)a.data)[i] ^= ((uint32_t*)b.data)[i];
                ((uint32_t*)b.data)[i] ^= ((uint32_t*)a.data)[i];
                ((uint32_t*)a.data)[i] ^= ((uint32_t*)b.data)[i];
            }
        }
        return;
    }
    if (dimensions <= 8) {
        for (int i=0;i<dimensions;i++) {
            if (swapBits&(1<<i)) {
                // swap part 1
                ((uint16_t*)a.data)[i] ^= ((uint16_t*)b.data)[i];
                ((uint16_t*)b.data)[i] ^= ((uint16_t*)a.data)[i];
                ((uint16_t*)a.data)[i] ^= ((uint16_t*)b.data)[i];
            }
        }
    }
}

double Wanderlust::calculateNewError(Pubkey &peer_pubkey, Location &myNewLocation, Location &peerNewLocation) {
    double newError = 0;
    for (std::map<Pubkey,WanderlustPeer>::iterator it=peers.begin();it!=peers.end();++it) {
        if (it->first != peer_pubkey) {
            newError += calculateDistance(myNewLocation, it->second.location)/peers.size();
        } else {
            newError += calculateDistance(myNewLocation, peerNewLocation)/peers.size();
        }
    }
    return newError;
}

void Wanderlust::snoopLocation(WanderlustHeader& header) {
    if (header.contents.src_pubkey == pubkey) return; // received a message from ourself?
    // store the source location, except if it's a swap response/confirmation
    if (header.contents.message_type == WANDERLUST_TYPE_SWAP_RESPONSE ||
        header.contents.message_type == WANDERLUST_TYPE_SWAP_CONFIRMATION) {
        locationStore[header.contents.src_pubkey] = header.contents.dst_location;
    } else {
        locationStore[header.contents.src_pubkey] = header.contents.src_location;
    }
}

void Wanderlust::SendPing(void) {
    if (locationStore.size() > 0) {
        // select a peer:
        map<Pubkey,Location>::iterator item = locationStore.begin();
        std::advance( item, rand()%locationStore.size());

        Ptr<Packet> packet = Create<Packet>();
        WanderlustHeader header;
        header.contents.src_location = location;
        header.contents.src_pubkey = pubkey;
        header.contents.dst_location = item->second;
        header.contents.dst_pubkey = item->first;
        header.contents.message_type = WANDERLUST_TYPE_PING;
        packet->AddHeader(header);
        route(packet, header);
        NS_LOG_DEBUG("sending ping " << sentPingCount << "/" << receivedPongCount << " " << locationStore.size() << " nodes in locationstore");
        sentPingCount++;
    }
    m_sendPingEvent = Simulator::Schedule(Seconds (5 + (rand()%1000)/100.0), &Wanderlust::SendPing, this);
}

/// Please make sure the header is already attached to the packet
void Wanderlust::route(Ptr<Packet> packet, WanderlustHeader& header) {
    if (peers.size() == 0) {
        NS_LOG_WARN("No peers so cannot route");
        return;
    }
    // find the closest peer FIXME: precompute, perhaps using sectors
    WanderlustPeer *best = NULL;
    double bestDistance = 0;
    bool direct = false;
    for (std::map<Pubkey,WanderlustPeer>::iterator it=peers.begin();it!=peers.end();++it) {
        if (it->first == header.contents.dst_pubkey) {
            // it's for this peer!
            direct = true;
            best = &it->second;
            break;
        }
        double thisDistance = calculateDistance(header.contents.dst_location, it->second.location);
        if (best == NULL || thisDistance < bestDistance) {
            bestDistance = thisDistance;
            best = &it->second;
        }
    }
    if (!direct && bestDistance > calculateDistance(header.contents.dst_location, location)) {
        // it's farther away than us, we should do local routing
        // drop it for now
        // FIXME: allow routing if this is a peer we have never gotten a packet destined for this address from
        NS_LOG_WARN("Dead end for " << header);
        return;
    }
    // and off we go!
    best->socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), 6556));
}

void Wanderlust::processPing(WanderlustPeer &peer, WanderlustHeader& header) {
    NS_LOG_DEBUG("received ping");
    // well, let's send something back!
    Ptr<Packet> packet = Create<Packet>();
    WanderlustHeader pongHeader;
    pongHeader.contents.src_location = location;
    pongHeader.contents.src_pubkey = pubkey;
    pongHeader.contents.dst_location = header.contents.src_location;
    pongHeader.contents.dst_pubkey = header.contents.src_pubkey;
    pongHeader.contents.message_type = WANDERLUST_TYPE_PONG;
    packet->AddHeader(pongHeader);
    route(packet, pongHeader);
}

void Wanderlust::processPong(WanderlustPeer &peer, WanderlustHeader& header) {
    receivedPongCount++;
    NS_LOG_DEBUG("received pong " << sentPingCount << "/" << receivedPongCount);
}


} // Namespace ns3
