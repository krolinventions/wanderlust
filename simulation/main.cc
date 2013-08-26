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
        runTime = 1000;
        nodeCount = 20;
        areaSize = 285*std::sqrt(nodeCount);

        LogComponentEnable ("WanderlustMain", LogLevel(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_LEVEL));
        //LogComponentEnable ("WanderlustApplication", LogLevel(LOG_LEVEL_WARN|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_LEVEL|LOG_PREFIX_FUNC));

        // To generate the topology we first place all nodes on a two dimensional map
        // the chance of an connection between two nodes is then inversely proportional to the distance
        NS_LOG_INFO ("Creating Topology...");
        // 50 looks enough for a good simulation
        NodeContainer nodes;
        nodes.Create (nodeCount);

        PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
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
        writeDotGraph2D();

        m_showLocationsEvent = Simulator::Schedule(Seconds (60), &MainObject::showLocations, this);
        applications.Start (Seconds (1.0));
        applications.Stop (Seconds (runTime));

        Simulator::Run ();

        showLocations();

        writeDotGraph();
        writeDotGraph2D();

        Simulator::Destroy ();
    }
    void showLocations() {
        double min;
        double max;
        double avg = 0;
        uint32_t pings = 0;
        uint32_t pongs = 0;
        for (unsigned int i=0;i<applications.GetN();i++) {
            Wanderlust &node = (Wanderlust&)*applications.Get(i);
            double error = node.getLocationError();
            //NS_LOG_INFO("Node " << i << " location " << ((Wanderlust&)*applications.Get(i)).getLocation() << " error " << error);
            avg += error/applications.GetN();
            if (i==0) {
                min = error;
                max = error;
            } else {
                min = std::min(error,min);
                max = std::max(error,max);
            }
            pings += node.getSentPingCount();
            pongs += node.getReceivedPongCount();
            node.resetStats();
        }
        cerr << Simulator::Now().GetSeconds() << "s min/avg/max " << min << "/" << avg << "/" << max << " ping/pong " << pings << "/" << pongs << " " << 100.0*pongs/pings << "%" << endl;
        if (Simulator::Now().GetSeconds() < runTime)
            m_showLocationsEvent = Simulator::Schedule(Seconds (60), &MainObject::showLocations, this);
    }
    void writeDotGraph() {
        cout << "graph {" << endl;
        for (uint i=0;i<applications.GetN();i++) {
            Wanderlust &node = (Wanderlust&)*applications.Get(i);
            double x = node.getX()*1.2;
            double y = node.getY()*1.2;
            cout << "    " << i << " [" << endl;
            cout << "        pos=\"" << x/100 << "," << y/100 << "!\"" << endl;
            cout << "        style=filled" << endl;
            cout << "        fillcolor=\"" << node.getLocation() << " 0.5 0.9\"" << endl;
            //cout << "        color=\"" << node.getLocation2() << " 0.5 0.9\"" << endl;
            cout << "        label=\"" << node.getLocationText() << "\"" << endl;
            //cout << "        penwidth=10" << endl;
            cout << "        shape=polygon" << endl;
            cout << "        distortion=-1.0" << endl;
            cout << "        sides=0" << endl;
            cout << "        orientation=" << node.getLocation2()*360 << endl;
            cout << "        fixedsize=true" << endl;
            cout << "        width=1" << endl;
            cout << "        height=1" << endl;
            cout << "    ]" << endl;
        }
        // connections
        for (uint i=0;i<connections.size();i++) {
            cout << "    " << connections[i].first << " -- " << connections[i].second << endl;
        }

        cout << "}" << endl;
    }
    void writeDotGraph2D() {
        // we draw the nodes 4 times, in one of the 4 quadrants
        // this way we can draw the surface of the torus without too much clutter
        cout << "graph {" << endl;
        for (int qx=0;qx<2;qx++) {
            for (int qy=0;qy<2;qy++) {
                for (uint i=0;i<applications.GetN();i++) {
                    Wanderlust &node = (Wanderlust&)*applications.Get(i);
                    double x = node.getLocation()*areaSize;
                    double y = node.getLocation2()*areaSize;
                    cout << "    \"" << i << "x" << qx << "y" << qy <<"\" [" << endl; // nnn_x_y
                    cout << "        pos=\"" << x/200*2 + areaSize/2*qx/100*2 << "," << y/200*2 + areaSize/2*qy/100*2 << "!\"" << endl;
                    cout << "        style=filled" << endl;
                    cout << "        fillcolor=\"" << node.getLocation() << " 0.5 0.9\"" << endl;
                    //cout << "        color=\"" << node.getLocation2() << " 0.5 0.9\"" << endl;
                    cout << "        label=\"" << node.getLocationText() << "\"" << endl;
                    //cout << "        penwidth=10" << endl;
                    cout << "        shape=polygon" << endl;
                    cout << "        distortion=-1.0" << endl;
                    cout << "        sides=0" << endl;
                    cout << "        orientation=" << node.getLocation2()*360 << endl;
                    cout << "        fixedsize=true" << endl;
                    cout << "        width=1" << endl;
                    cout << "        height=1" << endl;
                    cout << "    ]" << endl;
                }
            }
        }
        // connections
        for (uint i=0;i<connections.size();i++) {
            Wanderlust &node1 = (Wanderlust&)*applications.Get(connections[i].first);
            Wanderlust &node2 = (Wanderlust&)*applications.Get(connections[i].second);
            // we pick the instance of node1 that's in the center and connect it to the closest instance of node2

            int target1x = (node1.getLocation() < 0.5 ? 1 : 0);
            int target1y = (node1.getLocation2() < 0.5 ? 1 : 0);
            double node1x = node1.getLocation() + target1x;
            double node1y = node1.getLocation2() + target1y;
            double closest = 0;
            int target2x = -1;
            int target2y = -1;
            for (int qx=0;qx<2;qx++) {
                for (int qy=0;qy<2;qy++) {
                    double node2x = node2.getLocation() + qx;
                    double node2y = node2.getLocation2() + qy;
                    double distance = std::sqrt(std::pow(node1x-node2x,2)+std::pow(node1y-node2y,2));
                    if (target2x<0 || distance<closest) {
                        closest = distance;
                        target2x = qx;
                        target2y = qy;
                    }
                }
            }
            // we might be able to draw multiple edges
            // the things they share we can vary, the things they differ in we need to keep the same
            if (target1x == target2x && target1y == target2y) {
                // in the same quadrant, we can draw them all
                for (int qx=0;qx<2;qx++) {
                    for (int qy=0;qy<2;qy++) {
                        cout << "    \"" << connections[i].first << "x" << qx << "y" << qy << "\" -- \"" << connections[i].second << "x" << qx << "y" << qy << "\"" << endl;
                    }
                }
            } else if (target1x == target2x) {
                for (int qx=0;qx<2;qx++) {
                    cout << "    \"" << connections[i].first << "x" << qx << "y" << target1y << "\" -- \"" << connections[i].second << "x" << qx << "y" << target2y << "\"" << endl;
                }
            } else if (target1y == target2y) {
                for (int qy=0;qy<2;qy++) {
                    cout << "    \"" << connections[i].first << "x" << target1x << "y" << qy << "\" -- \"" << connections[i].second << "x" << target2x << "y" << qy << "\"" << endl;
                }
            } else {
                cout << "    \"" << connections[i].first << "x" << target1x << "y" << target1y << "\" -- \"" << connections[i].second << "x" << target2x << "y" << target2y << "\"" << endl;
            }
        }

        cout << "}" << endl;
    }

    int runTime;
    int nodeCount;
    int areaSize;
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
