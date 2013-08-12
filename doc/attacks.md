# Attacks
## Route table poisoning
Attacker performs a normal request to target and obtains a packet destined for evil node 1. He then sends this packet out of band (or encapsulated) to evil node 2. Evil node 2 starts sending his packet. All nodes on the route between evil node 2 and evil node 1 will now have the wrong route to the target. Evil node 2 can then drop all packets and prevent target from sending any.

Mitigation: Flow ID in packet, only use flow table for swap requests

## The happy swapper
An evil node generates a random location. When someone asks if he'd be happy to swap he responds that it would lower his energy massively. The other node is then at a worse location, and the evil node generates a new random location.

Mitigation: Only accept a swap if it will improve your own location. Convergence will be slower but nobody can force us to get a random location.

## One location to rule them all (Pitch black)
A evil node picks a fixed location (for example location 1). He then tries to swap, and if this succeedes he again reverts his own location to location 1. This way after a while the entire network will have the same location and routing will not work.

Mitigation: We need reject a swap if we detect another node is already using that location. Really close locations are not a problem as long as they are different.

## That's my location!
An evil node picks the same location as another node, to disrupt routing and convergence.

Mitigation: See location island

## Location island
A group of evil nodes all pick a location in a certain range around their target. They also keep reverting to an address in this range after a swap. This will cause the location gradient to move towards the group of evil nodes, disrupting routing until the target swaps his address with some node close to the evil group. Of course the evil group can then use the new location of their target. This attack is related to the Pitch Black attack.

Mitigation: We snoop on location swaps, and if a node somehow seems to move away from its optimal location (whether the optimal location changes or the location of the node) we blacklist it.

## Evil core router
A centrally connected node that passes a lot of traffic can decide to drop traffic from a certain target.

Mitigation: This will be discovered quickly and the evil node can be disconnected on the link level

## General Defense
For attacks that destabilize the network we should always be able to pinpoint the link the attacks come from. The attackers can then be tracked down on the link level and/or disconnected.

Interesting statistics:
- Number of packets (by type)
- Number of invalid packets
- Number of illogical packets (that we would expect to flow in the other direction based on the location)
- Average snooped distance from optimum
- Location stability (how often do we see nodes on this link change location)
- Illogical swap (node swaps to a location that would cause it to flow over another link)
