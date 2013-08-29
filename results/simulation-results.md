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
* ping addresses snooped (dst on response and confirmation)

Runs:

    * Locations 2D => ~15% reachability
    * Locations 1D => ~16% reachability
    * 2Di 15.9%

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
* ping addresses snooped (dst on response and confirmation)

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
* ping addresses snooped (dst on response and confirmation)

Runs:

    * 1D   => 47% 42s 27% 49% 50% avg 43.0% ****
    * 2D   => 22s 27s 30% 35% 33% avg 29.4% **
    * 2D i => 39% 55% 50% 55% 33% avg 46.4% ****
    * 3D i => 45% 46% 44% 39% 48% avg 44.4% ****
    * 4D i => 48% 31% 41% 41% 28% avg 37.5% ***

Conclusions: 2D with independent swapping seems best, followed by 3Di, but 1D is still good (and simple). This needs to be tested at an even bigger scale.

## Experiment 4
Parameters:
* 80 nodes
* 3960s
* no location mutation
* greedy routing
* direct routing to known neighbor
* random topology
* reachability averaged over 60s
* ping addresses snooped (dst on response and confirmation)

Runs:

    * 1D   => 18.5% 18.3%
    * 2D i => 24.3% 16.1%
    * 3D i => 23.7% 23.6%
    * 4D i => 25.5% 28.5%

Conclusions: based on the limited number of runs 4Di is looking good.

## Experiment 5

Parameters:
* 200 nodes
* 80 hours
* no location mutation
* greedy routing
* direct routing to known neighbor
* random topology
* ping addresses snooped (always source)

* 2Di => 13%? (infinite slowness)

## Experiment 6
Goal: check if we can get 100% reachability when all nodes are placed in a circle.

Parameters:
* 20 nodes
* 960s
* no location mutation
* greedy routing
* direct routing to known neighbor
* circular topology
* reachability averaged over 60s
* ping addresses snooped (ignore swap response, dst on confirmation)

    1D  => 23% 22% 26% 24% 32% avg 25.5% **
    2Di => 23% 34% 21% 22% 13% avg 22.6% **

Conclusion: the current algorithm will not even work correctly on a circle. This could either be because the time is too short to reach the optimum or it may have reached a lower optima where no swaps exist that are advantageous for both nodes.

## Experiment 7
Goal: see if taking more time will improve the results. Average time for reachability is also increased to 180s to reduce jitter.

Parameters:
* 20 nodes
* 1980s
* no location mutation
* greedy routing
* direct routing to known neighbor
* circular topology
* reachability averaged over 180s
* ping addresses snooped (ignore swap response, dst on confirmation)

    1D  => 17% 23% 18% 28% 26% avg 22.4% **

Conclusion: measurements from the same run show less variability (not shown here) but overall variation is not improved. Giving the algorithm more time does not seem to improve the results, however it can be seen during the run that there are still small changes in the locations. It is therefore deemed prudent to keep the time at the higher value.

## Experiment 8
Goal: see what we can do to improve reachability on a circle

Parameters:
* 20 nodes
* 1980s
* no location mutation
* greedy routing
* direct routing to known neighbor
* circular topology
* reachability averaged over 180s
* ping addresses snooped (ignore swap response, dst on confirmation)
* 1D locations

    always swap on response => 12% 17% 17% 12% 14% avg 14.4% (worse)
    mutate locations x       => 21% 17s 16% 31% 27s avg 22.4% (some slowness)
    mutate locations xn      => 12% 14% 12% 14%  8% avg 12.0% (no slowness)
    only 3 nodes            => 100% (always)
    only 4 nodes            => 84% 79% 82% 83% 100% avg 85.6%

n = no send back (fixed routing to prevent loops)
x = no actual mutation

Conclusion: Slowness may be caused by circular routing due to neighbors having old locations for each other. Extra loop prevention solves the slowness.  Location mutation seems to improve the reachability, but after fixing the routing loops this advantage seems to have disappeared. It is very suspicious that with only 4 nodes reachability is not 100%. There could be high packet loss between the nodes. Enabling some debugging shows nodes think the packet has reached a dead end.




