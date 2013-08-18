# Simulation
In order to see if the algorithm will work in practice we should first do some simulations. Nothing worse than developing software, convincing 10000 people to install it and only then finding out it the ideas behind it are faulty.

## Simulation Goals
Simulating Wanderlust should aim for the following goals:
- A working (simulated) implementation of the algorithm
- A framework that can easily simulate networks with different parameters (and with different tuning settings of the algorithm)
- The simulation should provide nice graphs and schematics in order to ease debugging and tuning, and in order to promote the project. Some animations would be especially nice.

## Simulation Frameworks
There are a number of existing frameworks which might be suitable for the simulation of the Wanderlust algorithm. They all have different advantages and disadvantages. The ICSI Networking and Security Group has a [nice list of network simulation tools](http://www.icir.org/models/simulators.html). These options will be reviewed to pick the one most suitable for Wanderlust.

### OMNeT++
> [OMNeT++][omnetpp] is an extensible, modular, component-based C++ simulation library and framework, primarily for building network simulators. "Network" is meant in a broader sense that includes wired and wireless communication networks, on-chip networks, queueing networks, and so on. Domain-specific functionality such as support for sensor networks, wireless ad-hoc networks, Internet protocols, performance modeling, photonic networks, etc., is provided by model frameworks, developed as independent projects. OMNeT++ offers an Eclipse-based IDE, a graphical runtime environment, and a host of other tools. There are extensions for real-time simulation, network emulation, alternative programming languages (Java, C#), database integration, SystemC integration, and several other functions.

This actually looks very interesting. I'm not too happy about the "Eclipse-based IDE" but it seems to produce nice graphs. Worth a try.

### pbsim
> [pbsim][] is a simulator written in Python that simulates the effects of the Pitch Black attack against Freenets Darknet location swapping algorithm.

The advantage is that it already contains code to generate a small world network and to test the DHT. Adding Wanderlust routing should be easy. This is quite a basic script and does not out of the box provide many analysis options.

### ns-3
> [ns-3][] is a discrete-event network simulator, targeted primarily for research and educational use. ns-3 is free software, licensed under the GNU GPLv2 license, and is publicly available for research, development, and use.

This also looks interesting. Hooking into the simulation seems to be more low level than OMNet++, which probably leads to better performance. It also provides good logging. It does not seem to produce as pretty graphs as OMNeT++. One of the advantages would be that this simulation can actually be connected to the real Wanderlust network, making bootstrapping it more interesting.

### Own implementation
It is also possible to write our own simulation tools. The advantages would be that the implementation of the routing algorithm for simulation can also be directly used in the implementation of the real network. Performance would almost certainly also be better than using one of the existing frameworks. Disadvantages are that it takes more work until the first simulation results are obtained, and producing nice graphs will also take more time.

[omnetpp]: http://www.omnetpp.org/ "OMNeT++"
[pbsim]: https://github.com/mgrube/pbsim "pbsim"
[ns-3]: http://www.nsnam.org/ "ns-3"

