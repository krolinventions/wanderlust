# Why the need for Wanderlust?
The organization of the current Internet is not centralized but very hierarchical. At the top stands IANA who hands out the IP addresses and local registrars keep subdividing the blocks until an end-user receives one or more addressed. Routing is also done hierarchically, with AS (autonomous systems) the biggest entities. The companies maintaining these autonomous systems work together to link them and provide worldwide connectivity. Such a hierarchical arrangement is necessary because there is no security inherent to the IPv4 or IPv6 protocols. Anyone could claim to own an address on the protocol level. The hierarchical layout allows the entities that manage the network to only allow the owners of the network to send packets from it, and also takes care to route return packets to the real owner. Another reason why the hierarchical structure works well is that it is much more economical to first gather all the flows and then transport them through high bandwidth connections and high bandwidth routers. Centralization makes the Internet faster, more reliable and cheaper.

If the current design of the Internet has so many good points, why would we want something different? The main reason is that governments and companies have a lot of power due to the centralized model. They can easily and cheaply block traffic they dislike or spy on their own citizens or those of another country. This can only be partially circumvented by the use of cryptography.

There currently are quite some projects which aim to address these points. There is [Freenet][] which operates like a giant [DHT][] and stores information anonymously on all participating nodes. Freenet is an established project and the network contains a lot of content. It is painfully slow however, and does not provide end-to-end connectivity, limiting the possible applications. Freenet can operate as a F2F darknet and provides its own routing.

There are also [Tor][] and [I2P][] which operate as a mix network for data streams or packets. Both Tor and I2P need the current Internet routing structure to function.

[CJDNS][] is actually the main inspiration for Wanderlust. However, we consider the routing algorithm (Kademlia without location swapping or changing connections) fundamentally broken. As described in the [paper by Oskar Sandberg][distroute] the greedy routing algorithm used by CJDNS (and Kademlia etc.) will only work if nodes that are closely connected also have close locations used for routing. Kademlia manages this by making and dropping connections, Freenet in darknet mode does this by swapping locations. CJDNS does neither and expect greedy routing to work. We expect that because the network is relatively small and everyone is connected through a small number of hops that every node has a route to every other node. In that case the switching code can take care of everything. Only after the switching label becomes full (the switching horizon) or if there is by chance no route available then packets need to be forwarded to another host. If this currently is a random host (not guaranteed to be closer to the target) it doesn't matter as much due to the small network size. Chances are that the other node will have a route, or it can forward to another random host. Eventually packets will get there, and because CJDNS is so quick that happens quickly. This will not scale to a worldwide network though, as the random routing will not find the target. Wanderlust is an attempt to take the main idea (hashes of public keys as IPv6 addresses) and add working routing on top of it. This is done as a separate project as the CJDNS code is very much intertwined with the label switching routing code.

There is also [GNUNet][] which works analogous to Freenet. It also seems to allow for IP tunneling though we could not find good documentation on how to do so. Also the F2F routing algorithm was not clearly described.

See also http://en.wikipedia.org/wiki/List_of_ad_hoc_routing_protocols

[distroute]: http://freenetproject.org/papers/swroute.pdf "Distributed Routing in Small-World Networks - Oskar Sandberg"
[2]: http://freenetproject.org/papers/vilhelm_thesis.pdf "Vilhelm Verendel"
[freenet]: https://freenetproject.org/ "Freenet, the free network"
[dht]: http://en.wikipedia.org/wiki/Distributed_hash_table "Distributed hash table"
[tor]: https://www.torproject.org/ "Tor"
[i2p]: http://www.i2p2.de/ "I2P Anonymous Network"
[kademlia]: http://en.wikipedia.org/wiki/Kademlia "Kademlia"
[cjdns]: http://cjdns.info/ "CJDNS"
[gnunet]: https://gnunet.org/ "GNUNet"
