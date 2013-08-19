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

#include "wanderlust-application.h"

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

Wanderlust::Wanderlust ()
{
  NS_LOG_FUNCTION (this);
}

Wanderlust::~Wanderlust()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
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

    if (m_socket == 0) {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket (GetNode (), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 6556);
        m_socket->Bind (local);
        m_socket->SetAllowBroadcast(true);
    }

    m_socket->SetRecvCallback (MakeCallback (&Wanderlust::HandleRead, this));

    m_sendEvent = Simulator::Schedule(Seconds (0.), &Wanderlust::SendSwapRequest, this);
}

void 
Wanderlust::StopApplication ()
{
    NS_LOG_FUNCTION (this);

    if (m_socket != 0) {
        m_socket->Close ();
        m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
    Simulator::Cancel (m_sendEvent);
}

void 
Wanderlust::HandleRead (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);

    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom (from)))
    {
        NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s node " << GetNode()->GetId() << " received " << packet->GetSize () << " bytes from " <<
            InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
            InetSocketAddress::ConvertFrom (from).GetPort ());
    }
}

void Wanderlust::SendSwapRequest(void) {
    NS_LOG_FUNCTION(this);
    
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s node " << GetNode()->GetId() <<  " sends a swaprequest packet");
    Ptr<Packet> p = Create<Packet>(1000);
    //m_txTrace (p); ??

    m_socket->SendTo(p,0,InetSocketAddress (Ipv4Address::GetBroadcast(), 6556));
    m_sendEvent = Simulator::Schedule(Seconds (1.), &Wanderlust::SendSwapRequest, this);
}

} // Namespace ns3
