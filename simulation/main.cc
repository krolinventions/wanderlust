/**
 * This file is part of Wanderlust.
 * 
 * To run the simulation:
 * 
 * ns-3.17$ ln -s ~/projects/meshnet/wanderlust/simulation scratch/wanderlust
 * ns-3.17$ ./waf --run wanderlust
 */
 
/*
 * Copyright (c) ???? ns-3 project
 * Copyright (c) 2013 Gerard Krol
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA    02111-1307    USA
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "wanderlust-helper.h"
#include "wanderlust-application.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WanderlustMain");

class MainObject {
public:
    void run() {
        LogComponentEnable ("WanderlustMain", LogLevel(LOG_LEVEL_INFO|LOG_PREFIX_TIME));
        //LogComponentEnable ("WanderlustApplication", LogLevel(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE));

        NS_LOG_INFO ("Creating Topology...");
        const int ringSize = 10;
        NodeContainer nodes;
        nodes.Create (ringSize);

        PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
        pointToPoint.SetChannelAttribute ("Delay", StringValue ("20ms"));

        InternetStackHelper stack;
        stack.Install (nodes);

        Ipv4AddressHelper address;
        address.SetBase ("10.0.0.0", "255.255.255.252");
        for (int i=0;i<ringSize;i++) {
            NetDeviceContainer c = pointToPoint.Install (nodes.Get(i), nodes.Get((i+1)%ringSize));
            address.Assign(c);
            address.NewNetwork();
        }

        WanderlustHelper wanderlustServer;

        serverApps = wanderlustServer.Install(nodes);
        m_showLocationsEvent = Simulator::Schedule(Seconds (10), &MainObject::showLocations, this);
        serverApps.Start (Seconds (1.0));
        serverApps.Stop (Seconds (runTime));

        Simulator::Run ();

        showLocations();

        Simulator::Destroy ();
    }
    void showLocations() {
        double min;
        double max;
        double avg;
        for (unsigned int i=0;i<serverApps.GetN();i++) {
            double error = ((Wanderlust&)*serverApps.Get(i)).getLocationError();
            NS_LOG_INFO("Node " << i << " location " << ((Wanderlust&)*serverApps.Get(i)).getLocation() << " error " << error);
            avg += error/serverApps.GetN();
            if (i==0) {
                min = error;
                max = error;
            } else {
                min = std::min(error,min);
                max = std::max(error,max);
            }
        }
        NS_LOG_INFO("min/avg/max " << min << "/" << avg << "/" << max);
        if (Simulator::Now().GetSeconds() < runTime)
            m_showLocationsEvent = Simulator::Schedule(Seconds (10), &MainObject::showLocations, this);
    }
    static const int runTime = 400;
    ApplicationContainer serverApps;
    EventId m_showLocationsEvent;
};

int main (int argc, char *argv[]) {
    srand(time(NULL));

    CommandLine cmd;
    cmd.Parse (argc, argv);

    MainObject m;
    m.run();

    return 0;
}
