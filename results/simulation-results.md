# Simulation Results

This page currently is a dumping ground for notes taken during simulations.

## Experiment 1
Goal: check difference between 1D and 2D for a larger amount of nodes and a bigger amount of time.

Parameters:
* 200 nodes
* 80 hours
* location mutation rand()%0x02000000 - 0x01000000 every 60-120 seconds
* greedy routing
* direct routing to known neighbor
* random topology

Runs:
* Locations 2D => ~15% reachability
* Locations 1D => ~16% reachability

Conclusion: no significant difference. Maybe when local routing is added this will change.

## Experiment 2
Goal: check difference between 1D, 2D and 64D on a lower timescale. Coordinates are circular unless otherwise indicated.

Parameters:
* 20 nodes
* 960s
* no location mutation
* greedy routing
* direct routing to known neighbor
* random topology
* reachability averaged over 60s

Runs:
* 0D   => 31% 31% note: simulation very slow (due to aimless wandering of packets?)
* 1D o => 40% 31% 33% 44% 41%
* 1D l => 68% 69% 88% 37% 45% average 62.4%
* 1D   => 44% 67% 74% 72% 72% average 65.5%
* 2D   => 71% 51% 59% 69% 35% average 57.0%
* 2D l => 56% 53% 44% 36% 62%
* 3D   => 41% 76% 50% 65% 40%
* 4D   => 48% 56% 54% 29% 39%
* 64D  => 58% 34% 38% 52% 34%
* 128D => 60% 30% 42% 57% 41% note: sometimes slow
* Kademlia 64 bit linear => 52% 61% 39% 41% 69%
* Kademlia 64 bit log    => 52% 61% 21% 70% 46%

l = linear
o = one direction only
