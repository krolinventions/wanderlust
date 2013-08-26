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
* ping addresses snooped

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
* ping addresses snooped

Runs:
    * 0D                     => 31% 31%                       *** note: simulation very slow (due to aimless wandering of packets?)
    * 1D o                   => 40% 31% 33% 44% 41% avg 37.8% ***
    * 1D l                   => 68% 69% 88% 37% 45% avg 62.4% ******
    * 1D                     => 44% 67% 74% 72% 72% avg 65.5% ******
    * 2D                     => 71% 51% 59% 69% 35% avg 57.0% *****
    * 2D l                   => 56% 53% 44% 36% 62% avg 50.2% *****
    * 2D i2                  => 70% 66% 49% 63% 48% avg 59.2% *****
    * 2D i3                  => 60% 92% 52% 56% 64% avg 64.8% ******
    * 3D                     => 41% 76% 50% 65% 40% avg 54.4% *****
    * 3D i2                  => 69% 72% 76% 66% 51% avg 66.7% ******
    * 3D i3                  => 64% 51% 73% 82% 63% avg 66.6% ******
    * 4D                     => 48% 56% 54% 29% 39% avg 45.2% ****
    * 4D i2                  => 80% 79% 45% 67% 73% avg 68.8% ******
    * 4D i3                  => 69% 78% 67% 62% 63% avg 67.8% ******
    * 5D i2                  => 50% 71% 68% 61% 79% avg 65.8% ******
    * 64D                    => 58% 34% 38% 52% 34% avg 43.2% ****
    * 128D                   => 60% 30% 42% 57% 41% avg 46.0% **** note: sometimes slow
    * Kademlia 64 bit linear => 52% 61% 39% 41% 69% avg 52.4% *****
    * Kademlia 64 bit log    => 52% 61% 21% 70% 46% avg 50.0% *****

l = linear
o = one direction only
i = dimensions independently swapped
2 = only change swapbits on request
3 = also change swapbits on response (incorrect?)

Conclusions: 0D and 1Do perform really bad. The 64D and the 128D aren't that good either. Best are the simple 1D and the runs where the dimensions were independently swapped. Those warrant further investigation.

## Experiment 3
Parameters:
* 40 nodes
* 1980s
* no location mutation
* greedy routing
* direct routing to known neighbor
* random topology
* reachability averaged over 60s
* ping addresses snooped

Runs:
    * 1D   => 47% 42s 27% 49% 50% avg 43.0% ****
    * 2D   => 22s 27s 30% 35% 33% avg 29.4% **
    * 2D i => 39% 55% 50% 55% 33% avg 46.4% ****
    * 3D i => 45% 46% 44% 39% 48% avg 44.4% ****
    * 4D i => 48% 31% 41% 41% 28% avg 37.5% ***

Conclusions: 2D with independent swapping seems best, followed by 3Di, but 1D is still good (and simple). This needs to be tested at an even bigger scale.

## Experiment 3
Parameters:
* 80 nodes
* 3960s
* no location mutation
* greedy routing
* direct routing to known neighbor
* random topology
* reachability averaged over 60s
* ping addresses snooped

Runs:
    * 1D   => 18.5% 18.3%
    * 2D i => 24.3% 16.1%
    * 3D i => 23.7% 23.6%
    * 4D i => 25.5% 28.5%

Conclusions: based on the limited number of runs 4Di is looking good.
