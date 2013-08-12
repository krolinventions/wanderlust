# Routing Algorithm
The physical network is a meshnet and consists of nodes that have one or more connections to other nodes. The way in which the connection is achieved is not important. This could be a direct (Ethernet) link, wireless or a VPN connection.

The goal is end-to-end connectivity, just like the real Internet. Just like it is expected on the normal Internet IP addresses should be impossible to spoof. By keeping the basics the same the transition will be much easier.

On startup Wanderlust generates a random location for itself in the spirit of [Kademlia][]. It then starts routing and performs the following periodic actions:
- A location swap is attempted
- The location of this node is advertised using DHT and to directly connected peers

# Routing
For routing it is assumed that the closer the locations of two nodes are, the less hops are between them. This implies that the shortest route to a certain location goes through the (connected) peer which has the location closest to the target. This is described in [Distributed Routing in Small-World Networks - Oskar Sandberg][distroute]

For routing Wanderlust maintains two different routing tables. The first one is the flow table. If a location swap packet is received the peer who forwarded it to us is added as the next hop for the source address so replies can be forwarded. After a while the flow times out and is removed.

The second table is a location table and this is part of the DHT which maps address -> location. Most Wanderlust routers are expected take part in this DHT. DHT entries can be either "stored" or "cached". "stored" entries persists until the local table is full and it's evicted and "cached" entries are removed using a timeout.

Nodes can opt not to take part in the DHT and only forward those messages. Those nodes are called Relays.

When we don't have an entry in the routing table for a locally sourced packet (for which we don't yet know the location of the destination) we buffer it and query the DHT. If no entry is found we add a blackhole route to the routing table, otherwise we add the destination location to the packet and send it using the normal routing.

# Location Swap
In order for the routing to work we must make sure that nodes that are physically close to each other (few hops) also have locations that are close to each other. The way to achieve this is to pick a node at random and see if we can both improve our locations by swapping them. A swap is performed using a three-way-handshake consisting of a request, a response and a confirmation. The request is routed randomly. The response is only sent if the node wants to participate in the swap and is periodically resent until the confirmation is received.

A Swap Request message is forwarded randomly (but not back to the peer from which it came). The TTL should be fairly large so it could end up anywhere in the network. Freenet uses 6 hops. The request contains our current and our ideal location so the receiver knows if the swap is going to be advantageous for us. After a random walk of 6 hops the packet could end up nearly anywhere in the network.

When a host receives or forwards such a message he adds an entry to the flow table. This is so we can easily send or forward reply packets. When the TTL reaches zero the packet is considered "received". If the swap is advantageous for both nodes (both end up closer to their ideal location) this node adopts the location of the other one and sends a Swap Response message.

When the packet reaches the node that sent the original request that node adopts the location of the other node and sends a confirmation.

Location energy is determined by the log sum of the path distances [2]. A lower energy is better. Note: Freenet currently uses a circle.
