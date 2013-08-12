# Routing Algorithm
The physical network is a meshnet and consists of nodes that have one or more connections to other nodes. The way in which the connection is achieved is not important. This could be a direct (Ethernet) link, wireless or a VPN connection.

The goal is end-to-end connectivity, just like the real Internet. Just like it is expected on the normal Internet IP addresses should be impossible to spoof. By keeping the basics the same the transition will be much easier.

Every Wanderlust node has:
- A public/private keypair
- An IPv6 addres derived from the public key
- A location

The keypair and IPv6 address are generated once and stay fixed, the location changes.

On startup Wanderlust generates a random location for itself in the spirit of [Kademlia][]. It then:
- Routes packets
- Periodically swaps his location with other nodes
- Periodically advertises its public key and location in the DHT

# Routing
Every data packet traveling through the network has a source address, a source location, a destination address and a destination location. Routing uses the location. It is assumed that the closer the locations of two nodes are, the less hops are between them. This implies that the shortest route to a certain location goes through the (connected) peer which has the location closest to the target. This is described in [Distributed Routing in Small-World Networks - Oskar Sandberg][distroute] This means a packet is forwarded to the directly connected peer who has the location closest to the destination.

For routing location swap messages Wanderlust maintains a different routing table, as in this case we cannot assume that the network has organized itself enough. This is the flow table. If a location swap packet is received the peer who forwarded it to us is added as the next hop for the source address so replies can be forwarded. After a while the flow times out and is removed.

If we need to send a packet to a node for which only the IPv6 address is know we first need to query the DHT for the public key and the location of this node. For this the location table is used. This table is part of the DHT which maps address -> location. Most Wanderlust routers are expected take part in this DHT. If a node doesn't have the IPv6 address in its location table it asks it to the peer with the closest location to it. One of the interesting things is that DHT keys are hashed into the same location space as the IPv6 addresses. Due to this DHT queries can be routed as if they were data packets. While we do not yet know the location and public key of a destination packets are buffered.

Nodes can opt not to take part in the DHT and only forward those messages. Those nodes are called Relays.

# Location Swap
In order for the routing to work we must make sure that nodes that are physically close to each other (few hops) also have locations that are close to each other. The way to achieve this is to pick a node at random and see if we can both improve our locations by swapping them. A swap is performed using a three-way-handshake consisting of a request, a response and a confirmation.

A Swap Request message is forwarded randomly (but not back to the peer from which it came). The TTL should be fairly large so it could end up anywhere in the network. Freenet uses 6 hops. The request contains our current and our ideal location so the receiver knows if the swap is going to be advantageous for us. After a random walk of 6 hops the packet could end up nearly anywhere in the network. When the TTL reaches zero the packet is considered "received". If the swap is advantageous for both nodes (both end up closer to their ideal location) this node adopts the location of the other one and sends a Swap Response message. If the swap would make the location any node worse the packet is dropped.

When the packet reaches the node that sent the original request that node adopts the location of the other node and sends a confirmation.

Location energy is determined by the log sum of the path distances [2]. A lower energy is better. Note: Freenet currently uses locations placed on a circle.

[distroute]: http://freenetproject.org/papers/swroute.pdf "Distributed Routing in Small-World Networks - Oskar Sandberg"
[2]: http://freenetproject.org/papers/vilhelm_thesis.pdf "Vilhelm Verendel"
[freenet]: https://freenetproject.org/ "Freenet, the free network"
[dht]: http://en.wikipedia.org/wiki/Distributed_hash_table "Distributed hash table"
[tor]: https://www.torproject.org/ "Tor"
[i2p]: http://www.i2p2.de/ "I2P Anonymous Network"
[kademlia]: http://en.wikipedia.org/wiki/Kademlia "Kademlia"
[cjdns]: http://cjdns.info/ "CJDNS"
[gnunet]: https://gnunet.org/ "GNUNet"
