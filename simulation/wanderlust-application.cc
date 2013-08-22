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

    // We enumerate all our network devices and bind a socket to every one of them
    for (unsigned int i=0;i<GetNode()->GetNDevices();i++) {
        Ptr<NetDevice> device = GetNode()->GetDevice(i);
        if (!device->GetChannel()) continue; // it's not connected
        NS_LOG_INFO ( "Node " << GetNode()->GetId() << " Found network device " << device->GetAddress() << " channel " << device->GetChannel());
        
        Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress(ipv4->GetInterfaceForDevice(device),0);
        
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        Ptr<Socket> socket = Socket::CreateSocket (GetNode (), tid);
        InetSocketAddress local = InetSocketAddress (iaddr.GetLocal(), 6556);
        int result = socket->Bind(local);
        socket->BindToNetDevice(device);
        NS_LOG_INFO ( "Binding to " << InetSocketAddress::ConvertFrom (local).GetIpv4 () << " result " << result << " bound to " << socket->GetBoundNetDevice());
        socket->SetAllowBroadcast(true);
        socket->SetRecvCallback (MakeCallback (&Wanderlust::HandleRead, this));
        sockets.push_back(socket);
    }

    m_sendSwapRequestEvent = Simulator::Schedule(Seconds (0.), &Wanderlust::SendSwapRequest, this);
    m_sendHelloEvent = Simulator::Schedule(Seconds (0.), &Wanderlust::SendHello, this);
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
        NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s node " << pubkey.getShortId() << " received " << packet->GetSize () << " bytes from " <<
            InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
            InetSocketAddress::ConvertFrom (from).GetPort ());
        WanderlustHeader header;
        packet->RemoveHeader(header);
        NS_LOG_INFO (header);
        switch (header.contents.message_type) {
            case WANDERLUST_TYPE_DATA: break;
            case WANDERLUST_TYPE_SWAP_REQUEST: break;
            case WANDERLUST_TYPE_SWAP_RESPONSE: break;
            case WANDERLUST_TYPE_SWAP_CONFIRMATION: break;
            case WANDERLUST_TYPE_LOCATION_QUERY: break;
            case WANDERLUST_TYPE_LOCATION_ANSWER: break;
            case WANDERLUST_TYPE_HELLO:
                // we've received an awesome hello packet
                // hello other node! want to be friends?
                // well, let's add it to our list
                if (peers.count(header.contents.src_pubkey) == 0) {
                    // we don't have it yet, create a new one
                    peers[header.contents.src_pubkey] = WanderlustPeer();
                }
                // update the location
                peers[header.contents.src_pubkey].location = header.contents.src_location;
                peers[header.contents.src_pubkey].socket = socket;
                break;
            default:
                break;
        }
    }
}

void Wanderlust::SendSwapRequest(void) {
    NS_LOG_FUNCTION(this);
    
    for (std::map<pubkey_t,WanderlustPeer>::iterator it=peers.begin();it!=peers.end();++it) {
        WanderlustPeer &peer = it->second;

        Ptr<Packet> p = Create<Packet>();

        WanderlustHeader header;
        header.contents.message_type = WANDERLUST_TYPE_SWAP_REQUEST;
        header.contents.src_pubkey = pubkey;
        header.contents.src_location = location;
        header.contents.hop_limit = 1;
        p->AddHeader(header);

        NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s node " << pubkey.getShortId() <<  " sends a swaprequest packet " << header);
        peer.socket->SendTo(p,0,InetSocketAddress (Ipv4Address::GetBroadcast(), 6556));
    }
    m_sendSwapRequestEvent = Simulator::Schedule(Seconds (1.), &Wanderlust::SendSwapRequest, this);
}

void Wanderlust::SendHello(void) {
    NS_LOG_FUNCTION(this);

    for (std::vector< Ptr<Socket> >::iterator it=sockets.begin();it!=sockets.end();++it) {
        Ptr<Socket> socket = *it;
        NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s node " << pubkey.getShortId() <<  " sends a hello packet");
        Ptr<Packet> p = Create<Packet>();
        //m_txTrace (p); ??
        WanderlustHeader header;
        header.contents.message_type = WANDERLUST_TYPE_HELLO;
        header.contents.src_pubkey = pubkey;
        header.contents.src_location = location;
        p->AddHeader(header);
        socket->SendTo(p,0,InetSocketAddress (Ipv4Address::GetBroadcast(), 6556));
    }
    m_sendHelloEvent = Simulator::Schedule(Seconds (1.), &Wanderlust::SendHello, this);
}

} // Namespace ns3
