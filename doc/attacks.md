# Attacks
If someone would want to disrupt the network there are a number of different attacks he might attempt. Some of these things might even happen naturally during operation of the network. Attacks can be split into three parts: wanderlust specific, location related and routing related. The last two attack types are shared with Freenet.

## Wanderlust specific

### Route table poisoning
Attacker performs a normal request to target and obtains a packet destined for evil node 1. He then sends this packet out of band (or encapsulated) to evil node 2. Evil node 2 starts sending his packet. All nodes on the route between evil node 2 and evil node 1 will now have the wrong route to the target. Evil node 2 can then drop all packets and prevent target from sending any.

Mitigation: Flow ID in packet, only use flow table for swap requests

## Location related (Pitch Black)
A good paper on the weaknesses of location based routing are described in the paper [Routing in the Dark: Pitch Black][pitchblack] by Evans et al.

### The happy swapper
An evil node generates a random location. When someone asks if he'd be happy to swap he responds that it would lower his energy massively. The other node is then at a worse location, and the evil node generates a new random location.

Mitigation: Only accept a swap if it will improve your own location. Convergence will be slower but nobody can force us to get a random location.

### One location to rule them all
A evil node picks a fixed location (for example location 1). He then tries to swap, and if this succeeds he again reverts his own location to location 1. This way after a while the entire network will have the same location and routing will not work.

Mitigation: We need reject a swap if we detect another node is already using that location. Really close locations are not a problem as long as they are different.

### That's my location!
An evil node picks the same location as another node, to disrupt routing and convergence.

Mitigation: See location island, Single act node

### Location island
A group of evil nodes all pick a location in a certain range around their target. They also keep reverting to an address in this range after a swap. This will cause the location gradient to move towards the group of evil nodes, disrupting routing until the target swaps his address with some node close to the evil group. Of course the evil group can then use the new location of their target. This attack is related to "That's my location".

Mitigation: We snoop on location swaps, and if a node somehow seems to move away from its optimal location (whether the optimal location changes or the location of the node) we blacklist it.

### Sybil island
A single attacker pretends to be a large bunch of nodes. He makes all those nodes pick a location close to the target and lets normal network optimization play out. This will cause routing for the target to shift towards to attacker. As he does not need to revert any locations monitoring swap request will not help.

Mitigation: We monitor the location distribution of never seen before nodes on a link after an initial warmup period. If this location distribution is skewed too much we cut the link.

### Single act node
If a node joins the network for the first time, swaps his location with another node that wants it and then leaves he will have reduced the entropy in the available addresses. The node he swapped with agreed to the swap because he got a location closer to his neighbors. The node that leaves takes the address that balances the swap with it.

Mitigation: In general these attacks work by reducing the entropy in the node locations. We need to make sure we keep this high enough. This can be done by periodically and randomly modifying a nodes location a little bit. The small change will not significantly impact routing but the added entropy will hopefully keep the network healthy.

We should limit the number of allowed swaps (and new nodes) so that the amount of entropy that an attacker can reduce with the attack is lower than the amount we can generate by changing the locations. Of course we still need to allow for enough swaps to self-organize the network. This is an interesting balance, and whether this is possible should be provable.

Mitigation #2 (by Freenet): find out how close nodes are typically (pick a random key, do a probe, take the average over 5 random keys), if your peers are a lot closer than that then randomise. Also see the [Freenet bug for this](https://bugs.freenetproject.org/view.php?id=3919).

## Forwarding related

### Evil core router
A centrally connected node that passes a lot of traffic can decide to drop traffic from or to a certain target. This does not have to be all traffic, a packet loss of 10% would already be very annoying.

Mitigation: This is actually quite hard to determine, as the attacker can invent any number of surrounding nodes (sybil) and let those take the blame. We also don't want an attacker to be able to determine the network topology. FIXME

# General Defense
For attacks that destabilize the network we should always be able to pinpoint the link the attacks come from. The attackers can then be tracked down on the link level and/or disconnected.

Interesting statistics:
- Number of packets (by type)
- Number of invalid packets
- Number of illogical packets (that we would expect to flow in the other direction based on the location)
- Average snooped distance from optimum
- Location stability (how often do we see nodes on this link change location)
- Illogical swap (node swaps to a location that would cause it to flow over another link)

[pitchblack]: http://grothoff.org/christian/pitchblack.pdf "Routing in the Dark: Pitch Black"
