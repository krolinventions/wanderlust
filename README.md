# Wanderlust
Wanderlust - A Worldwide Network with Distributed Routing through Location Swapping

The centralized nature of the current Internet allows for easy censoring and invasion of privacy. Current technical solutions are not sufficient. Wanderlust is a decentralized overlay/mesh network compatible with existing IPv6 aware applications that aims to fix this. Nodes have an IPv6 addresses which is created from their public key. A node connects to a number of peers over the normal Internet or through private links. The connections between nodes are assumed to form a small world network and location swapping is used to assign every node a location that is close to its directly connected peers. Using the location gradient packets can then be routed to nodes. A DHT provides a global location lookup for any address. Encryption is used throughout the network to provide privacy and prevent attacks on the network.

## Goals
This project aims for the following:
- A routing algorithm that allows for efficient decentralized routing.
- A protocol that allows for the communication needed for the implementation of the algorithm and for the transfer of the information itself, safely and securely.
- The design and implementation of an application that implements the routing algorithm and the protocol.
- A global network of nodes that can communicate without fear of censorship or privacy violations.

## Status
The algorithm is done and protocol design is taking shape. No code has yet been written. It is possible that Wanderlust could be most easily implemented as a component of Cjdns or as a fork.

1. Routing algorithm design - completed
2. Protocol design - in progress
3. Software design
4. Tunnel: two nodes, packets in, packets out, only type 0 messages
5. Proto-Relay: three nodes, switching based on directly connected address
6. Relay: proto-relay with location swapping
7. Node: relay + DHT for global routing
8. Grow the network

## Contributing
Currently its most important to get the design clear and without flaws. You can help by asking questions if things are unclear using the Github bug tracker. Also, if you can think of situations in which the system might not function optimally or of possible attacks please let us know as soon as possible.

We do not yet have a mailing list, this is being worked on.

You are also encouraged to fork this repository and make any changes that you think are improvements. Please send a pull request if you think your work is ready for inclusion. Corrections of spelling and grammar definitely count as improvements.

## Read more
Please read the following if you want to know more about the design or implementation of Wanderlust.

- [Why the need for Wanderlust?](doc/why-wanderlust.md)
- [Routing](doc/routing.md)
- [Protocol](doc/protocol.md)
- [Attacks](doc/attacks.md)

