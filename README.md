# Wanderlust
A Worldwide Network with Distributed Routing through Location Swapping - Gerard Krol, 2013

## Abstract
The centralized nature of the current Internet allows for easy censoring and invasion of privacy. Current technical solutions are not sufficient. We propose Wanderlust, a decentralized overlay network compatible with existing IPv6 aware applications. Nodes have an IPv6 addresses which is created from their public key. A node connects to a number of peers over the normal Internet or through private links. The connections between nodes are assumed to form a small world network and location swapping is used to assign every node a location that is close to its directly connected peers. Using the location gradient packets can then be routed to nodes. A DHT provides a global location lookup for any address. Encryption is used throughout the network to provide privacy and prevent attacks on the network.

## Status
The protocol design is taking shape, no code has yet been written. Please comment if this document is not clear enough, if you find any flaws/attacks or have other ideas for improvements.

Development Stages:

1. Protocol design - in progress
2. Software design
3. Tunnel: two nodes, packets in, packets out, only type 0 messages
4. Proto-Relay: three nodes, switching based on directly connected address
5. Relay: proto-relay with location swapping, test network of 20 nodes
6. Node: relay + DHT for global routing, test network of 1000 nodes

## Introduction
The organization of the current Internet is not centralized but very hierarchical. At the top stands IANA who hands out the IP addresses and local registrars keep subdividing the blocks until an end-user receives one or more addressed. Routing is also done hierarchically, with AS (autonomous systems) the biggest entities. The companies maintaining these autonomous systems work together to link them and provide worldwide connectivity. Such a hierarchical arrangement is necessary because there is no security inherent to the IPv4 or IPv6 protocols. Anyone could claim to own an address on the protocol level. The hierarchical layout allows the entities that manage the network to only allow the owners of the network to send packets from it, and also takes care to route return packets to the real owner. Another reason why the hierarchical structure works well is that it is much more economical to first gather all the flows and then transport them through high bandwidth connections and high bandwidth routers. Centralization makes the Internet faster, more reliable and cheaper.

If the current design of the Internet has so many good points, why would we want something different? The main reason is that governments and companies have a lot of power due to the centralized model. They can easily and cheaply block traffic they dislike or spy on their own citizens or those of another country. This can only be partially circumvented by the use of cryptography.

There currently are quite some projects which aim to address these points. There is [Freenet][] which operates like a giant [DHT][] and stores information anonymously on all participating nodes. Freenet is an established project and the network contains a lot of content. It is painfully slow however, and does not provide end-to-end connectivity, limiting the possible applications. Freenet can operate as a F2F darknet and provides its own routing.

There are also [Tor][] and [I2P][] which operate as a mix network for data streams or packets. Both Tor and I2P need the current Internet routing structure to function.

[CJDNS][] is actually the main inspiration for Wanderlust. However, we consider the routing algorithm (Kademlia without location swapping or changing connections) fundamentally broken. As described in the [paper by Oskar Sandberg][distroute] the greedy routing algorithm used by CJDNS (and Kademlia etc.) will only work if nodes that are closely connected also have close locations used for routing. Kademlia manages this by making and dropping connections, Freenet in darknet mode does this by swapping locations. CJDNS does neither and expect greedy routing to work. We expect that because the network is relatively small and everyone is connected through a small number of hops that every node has a route to every other node. In that case the switching code can take care of everything. Only after the switching label becomes full (the switching horizon) or if there is by chance no route available then packets need to be forwarded to another host. If this currently is a random host (not guaranteed to be closer to the target) it doesn't matter as much due to the small network size. Chances are that the other node will have a route, or it can forward to another random host. Eventually packets will get there, and because CJDNS is so quick that happens quickly. This will not scale to a worldwide network though, as the random routing will not find the target. Wanderlust is an attempt to take the main idea (hashes of public keys as IPv6 addresses) and add working routing on top of it. This is done as a separate project as the CJDNS code is very much intertwined with the label switching routing code.

There is also [GNUNet][] which works analogous to Freenet. It also seems to allow for IP tunneling though we could not find good documentation on how to do so. Also the F2F routing algorithm was not clearly described.

See also http://en.wikipedia.org/wiki/List_of_ad_hoc_routing_protocols

## Operation

The physical network is a meshnet and consists of nodes that have one or more connections to other nodes. The way in which the connection is achieved is not important. This could be a direct (Ethernet) link, wireless or a VPN connection.

The goal is end-to-end connectivity, just like the real Internet. Just like it is expected on the normal Internet IP addresses should be impossible to spoof. By keeping the basics the same the transition will be much easier.

On startup Wanderlust generates a random location for itself in the spirit of [Kademlia][]. It then starts routing and performs the following periodic actions:
- A location swap is attempted
- The location of this node is advertised using DHT and to directly connected peers

### Routing
For routing it is assumed that the closer the locations of two nodes are, the less hops are between them. This implies that the shortest route to a certain location goes through the (connected) peer which has the location closest to the target. This is described in [Distributed Routing in Small-World Networks - Oskar Sandberg][distroute]

For routing Wanderlust maintains two different routing tables. The first one is the flow table. If a location swap packet is received the peer who forwarded it to us is added as the next hop for the source address so replies can be forwarded. After a while the flow times out and is removed.

The second table is a location table and this is part of the DHT which maps address -> location. Most Wanderlust routers are expected take part in this DHT. DHT entries can be either "stored" or "cached". "stored" entries persists until the local table is full and it's evicted and "cached" entries are removed using a timeout.

Nodes can opt not to take part in the DHT and only forward those messages. Those nodes are called Relays.

When we don't have an entry in the routing table for a locally sourced packet (for which we don't yet know the location of the destination) we buffer it and query the DHT. If no entry is found we add a blackhole route to the routing table, otherwise we add the destination location to the packet and send it using the normal routing.

### Location Swap
In order for the routing to work we must make sure that nodes that are physically close to each other (few hops) also have locations that are close to each other. The way to achieve this is to pick a node at random and see if we can both improve our locations by swapping them. A swap is performed using a three-way-handshake consisting of a request, a response and a confirmation. The request is routed randomly. The response is only sent if the node wants to participate in the swap and is periodically resent until the confirmation is received.

A Swap Request message is forwarded randomly (but not back to the peer from which it came). The TTL should be fairly large so it could end up anywhere in the network. Freenet uses 6 hops. The request contains our current and our ideal location so the receiver knows if the swap is going to be advantageous for us. After a random walk of 6 hops the packet could end up nearly anywhere in the network.

When a host receives or forwards such a message he adds an entry to the flow table. This is so we can easily send or forward reply packets. When the TTL reaches zero the packet is considered "received". If the swap is advantageous for both nodes (both end up closer to their ideal location) this node adopts the location of the other one and sends a Swap Response message.

When the packet reaches the node that sent the original request that node adopts the location of the other node and sends a confirmation.

Location energy is determined by the log sum of the path distances [2]. A lower energy is better. Note: Freenet currently uses a circle.

## Protocol
This section describes the layout of the different packets. Cryptography is done using [Elliptic Curve Cryptography][ecc] (ECC) with 256 bit keys. The choice for ECC is mainly due to the smaller key sizes, leading to less overhead. The suspicion that RSA may soon be severely weakened or broken also plays a role.

IPv6/IPv4 header (40 bytes)
UDP header (8 bytes)
LSDRP header:
- version (1 byte) - if there is no explicit support for this version the packet should be dropped
- header length (1 byte) - in 8 byte units
- message type (1 byte) - message type
- hop limit (1 byte) - hop limit
- flags (1 byte) - bit 1: is payload encrypted?
- flow id (3 bytes) - used for routing swap packets

- source pubkey (256 bits/32 bytes)
- destination pubkey (256 bits/32 bytes)
- source location (128 bit/16 bytes)
- destination location (128 bit/16 bytes)

- Signature using source privkey (of entire packet with signature field zero, 64 bytes)

- Message contents possibly encrypted with destination pubkey (only encrypted, not signed)

Note: The first byte of a location is reserved for the location type for future experiments and should be zero.

The header (pubkeys + location) could optionally be encrypted but this makes plain switching duties much more expensive.

### Message 0: data packet
- Encrypted payload:
- Original IPv6 packet with source and destination address removed (can be derived from pubkeys)
- The total overhead is: 40 + 8 + 4 + 32*2 + 16*2 + 64 -2*32= 148 bytes overhead (ipsec = 56)

### Message 1: swap request
- source information filled
- destination information zero -> randomly routed
- hop limit around 6
- Payload: optimal location (128 bits)

### Message 2: swap response
- Drop if your location is not closer to the optimal location from the requester
- Note: resend until you receive a confirmation, then perform the swap
- Payload: optimal location (128 bits)

### Message 3: swap confirmation
- No payload

### Message 4: location query
- Note: destination location is the hash of the address

### Message 5: location answer
- Unencrypted payload
- Pubkey
- Location
- Timestamp (64 bit UTC)
- Signature
- Extra fields
Note: can be used for inserts with zero address, with as destination location the hashed address

### Later:
Message 6: Ping
- Can be sent to the zero address (to only a location)
- Can be used to check if a location is in use

Message 7: Pong

Message 8: Dead-end
Message 9: HTL exceeded
Message 10: Invalid packet

### Receiving a packet
- Invalid packets (Type 1, destination address not zero) -> drop
- If type is 1 or 2 add address + flow id to flow table
- HTL 1, type 1, destination address zero -> process swap request
- Type 1, destination address zero -> switching
- Type 5 -> store location if newer
- We are not the destination -> switching

- Type 0 -> send to local interface
- Type 2 -> process swap response
- Type 3 -> process swap confirmation
- Type 4 -> answer if possible, otherwise switch
- Type 5, we are the destination -> drop silently (already processed)

- Log error

#### Switching
- Decrease HTL, drop if 0
- Destination address zero, location zero -> route to random other peer
- Destination address nonzero and in flow table -> forward to peer from table
- route to peer with closest location

## Attacks
### Route table poisoning
Attacker performs a normal request to target and obtains a packet destined for evil node 1. He then sends this packet out of band (or encapsulated) to evil node 2. Evil node 2 starts sending his packet. All nodes on the route between evil node 2 and evil node 1 will now have the wrong route to the target. Evil node 2 can then drop all packets and prevent target from sending any.

Mitigation: Flow ID in packet, only use flow table for swap requests

### The happy swapper
An evil node generates a random location. When someone asks if he'd be happy to swap he responds that it would lower his energy massively. The other node is then at a worse location, and the evil node generates a new random location.

Mitigation: Only accept a swap if it will improve your own location. Convergence will be slower but nobody can force us to get a random location.

### One location to rule them all
An evil node picks a fixed location (for example location 1). He then tries to swap, and if this succeedes he again reverts his own location to location 1. This way after a while the entire network will have the same location and routing will not work.

Mitigation: We need reject a swap if we detect another node is already using that location. Really close locations are not a problem as long as they are different.

### That's my location!
An evil node picks the same location as another node, to disrupt routing and convergence.

Mitigation: See location island

### Location island
A group of evil nodes all pick a location in a certain range around their target. They also keep reverting to an address in this range after a swap. This will cause the location gradient to move towards the group of evil nodes, disrupting routing until the target swaps his address with some node close to the evil group. Of course the evil group can then use the new location of their target.

Mitigation: We snoop on location swaps, and if a node somehow seems to move away from its optimal location (whether the optimal location changes or the location of the node) we blacklist it.

### Evil core router
A centrally connected node that passes a lot of traffic can decide to drop traffic from a certain target.

Mitigation: This will be discovered quickly and the evil node can be disconnected on the link level

### General
For attacks that destabilize the network we should always be able to pinpoint the link the attacks come from. The attackers can then be tracked down on the link level and/or disconnected.

Interesting statistics:
- Number of packets (by type)
- Number of invalid packets
- Number of illogical packets (that we would expect to flow in the other direction based on the location)
- Average snooped distance from optimum
- Location stability (how often do we see nodes on this link change location)
- Illogical swap (node swaps to a location that would cause it to flow over another link)

[distroute]: http://freenetproject.org/papers/swroute.pdf "Distributed Routing in Small-World Networks - Oskar Sandberg"
[2]: http://freenetproject.org/papers/vilhelm_thesis.pdf "Vilhelm Verendel"
[freenet]: https://freenetproject.org/ "Freenet, the free network"
[dht]: http://en.wikipedia.org/wiki/Distributed_hash_table "Distributed hash table"
[tor]: https://www.torproject.org/ "Tor"
[i2p]: http://www.i2p2.de/ "I2P Anonymous Network"
[kademlia]: http://en.wikipedia.org/wiki/Kademlia "Kademlia"
[cjdns]: http://cjdns.info/ "CJDNS"
[gnunet]: https://gnunet.org/ "GNUNet"
[ecc]: http://en.wikipedia.org/wiki/Elliptic_curve_cryptography "Elliptic Curve Cryptography"
