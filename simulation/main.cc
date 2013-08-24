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
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WanderlustMain");

class MainObject {
public:
    void run() {
        LogComponentEnable ("WanderlustMain", LogLevel(LOG_LEVEL_INFO|LOG_PREFIX_TIME));
        //LogComponentEnable ("WanderlustApplication", LogLevel(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE));

        // To generate the topology we first place all nodes on a two dimensional map
        // the chance of an connection between two nodes is then inversely proportional to the distance
        NS_LOG_INFO ("Creating Topology...");
        const int nodeCount = 50;
        const int areaSize = 2000;
        NodeContainer nodes;
        nodes.Create (nodeCount);

        PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
        pointToPoint.SetChannelAttribute ("Delay", StringValue ("20ms"));

        InternetStackHelper stack;
        stack.Install (nodes);

        Ipv4AddressHelper address;
        address.SetBase ("10.0.0.0", "255.255.255.252");

        WanderlustHelper wanderlustServer;
        applications = wanderlustServer.Install(nodes);

        for (int i=0;i<nodeCount;i++) {
            // set positions
            Wanderlust &node = (Wanderlust&)*applications.Get(i);
            double x = rand()%areaSize;
            double y = rand()%areaSize;
            node.setPosition(x, y); // in m
        }
        for (int i=0;i<nodeCount;i++) {
            Wanderlust &node1 = (Wanderlust&)*applications.Get(i);
            for (int j=i+1;j<nodeCount;j++) {
                Wanderlust &node2 = (Wanderlust&)*applications.Get(j);
                double distanceSquared = node1.calculateDistanceSquared(node2);
                // 0m -> 100%, 1000m -> 5%
                double probablility = std::exp(-5E-6*distanceSquared);
                //NS_LOG_INFO ("Node " << i << " and " << j << " probability " << probablilty);
                if (rand()/(double)RAND_MAX < probablility) {
                    connections.push_back(pair<uint32_t,uint32_t>(i,j));
                    NetDeviceContainer c = pointToPoint.Install (nodes.Get(i), nodes.Get(j));
                    address.Assign(c);
                    address.NewNetwork();
                }
            }
        }
        writeDotGraph();

        m_showLocationsEvent = Simulator::Schedule(Seconds (10), &MainObject::showLocations, this);
        applications.Start (Seconds (1.0));
        applications.Stop (Seconds (runTime));

        Simulator::Run ();

        showLocations();

        writeDotGraph();

        Simulator::Destroy ();
    }
    void showLocations() {
        double min;
        double max;
        double avg;
        for (unsigned int i=0;i<applications.GetN();i++) {
            double error = ((Wanderlust&)*applications.Get(i)).getLocationError();
            NS_LOG_INFO("Node " << i << " location " << ((Wanderlust&)*applications.Get(i)).getLocation() << " error " << error);
            avg += error/applications.GetN();
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
    void writeDotGraph() {
        cout << "graph {" << endl;
        for (uint i=0;i<applications.GetN();i++) {
            Wanderlust &node = (Wanderlust&)*applications.Get(i);
            double x = node.getX();
            double y = node.getY();
            cout << "    " << i << " [" << endl;
            cout << "        pos=\"" << x/100 << "," << y/100 << "!\"" << endl;
            cout << "        style=filled" << endl;
            cout << "        fillcolor=\"" << node.getLocation() << " 0.5 0.9\"" << endl;
            cout << "    ]" << endl;
        }
        // connections
        for (uint i=0;i<connections.size();i++) {
            cout << "    " << connections[i].first << " -- " << connections[i].second << endl;
        }

        cout << "}" << endl;
    }

    static const int runTime = 3600*3;
    ApplicationContainer applications;
    EventId m_showLocationsEvent;
    vector<pair<uint32_t, uint32_t> > connections;
};

int main (int argc, char *argv[]) {
    srand(time(NULL));

    CommandLine cmd;
    cmd.Parse (argc, argv);

    MainObject m;
    m.run();

    return 0;
}
