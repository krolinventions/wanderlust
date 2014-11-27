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

```
    1D  => 23% 22% 26% 24% 32% avg 25.5% **
    2Di => 23% 34% 21% 22% 13% avg 22.6% **
```

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

```
    1D  => 17% 23% 18% 28% 26% avg 22.4% **
```

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

```
    always swap on response => 12% 17% 17% 12% 14% avg 14.4% (worse)
    mutate locations x      => 21% 17s 16% 31% 27s avg 22.4% (some slowness)
    mutate locations xn     => 12% 14% 12% 14%  8% avg 12.0% (no slowness)
    only 3 nodes            => 100% (always)
    only 4 nodes            => 84% 79% 82% 83% 100% avg 85.6%
    4 nodes s f             => 99% 100% 99% 99% 100%  avg 99.4% (packet loss just after swap)
    4 nodes f               => 100% 63% 58% 100% 100% avg 84.2%
    f                       => 10% 10% (lots and lots of duplicate locations?) 
    5 s f                   => 100 100 100 100 100 avg 100%
    6 s f                   => 100 100 100 100 100 avg 100%
    10 s f                  => 19% 13% 24% 27% 100 avg 36.6
```

```
n = no send back (fixed routing to prevent loops)
x = no actual mutation
s = also swap when we get worse but it is a net improvement (kind)
f = fixed distance calc (fmod)
```

Conclusion: Slowness may be caused by circular routing due to neighbors having old locations for each other. Extra loop prevention solves the slowness.  Location mutation seems to improve the reachability, but after fixing the routing loops this advantage seems to have disappeared. It is very suspicious that with only 4 nodes reachability is not 100%. There could be high packet loss between the nodes. Enabling some debugging shows nodes think the packet has reached a dead end.

Kind swapping (with fixed distance calculation) seems to be the solution.

## Experiment 9
Goal: see at what circle size routing breaks down

Parameters:
* nodes varies
* 1980s
* no location mutation
* greedy routing
* direct routing to known neighbor
* circular topology
* reachability averaged over 180s
* ping addresses snooped (ignore swap response, dst on confirmation)
* 1D locations (fmod distance calc)
* kind swapping

```
    4   => 99% 100% 99% 99% 100%  avg 99.4% (packet loss just after swap)
    5   => 100 100 100 100 100 avg 100%
    6   => 100 100 100 100 100 avg 100%
    7   => 100 100 100 100 100 avg 100%
    8   => 100 100 100 100 27% avg 85.4%
    9   => 23% 27% 100 100 100 avg 70.0%
    10  => 19% 13% 24% 27% 100 avg 36.6%
```

Conclusion: these settings break down at circles > 7 nodes. When looking at the final map the ones with routing problems completely failed to converge. Maybe swapping packets don't travel far enough.

## Experiment 10
Goal: see if we can get 10 node circles to route

Parameters:
* 10 nodes
* 1980s
* no location mutation
* greedy routing
* direct routing to known neighbor
* circular topology
* reachability averaged over 180s
* ping addresses snooped (ignore swap response, dst on confirmation)
* 1D locations (fmod distance calc)
* kind swapping

```
    3960s            => 23% 99% 20% 24% 23% avg 37.8%
    swap request 99% => 100 25% 23% 19% 25% avg 38.4%
    swap request 90% => 26% 25% 23% 23% 21% avg 23.6%
    2D               => 18% 22% 18% 21% 25% avg 20.8%
    2Di              => 20% 15% 22% 19% 30%
    loc mut          => 26% 20% 19% 26% 22% avg 22.6%
```

Conclusion: increasing the time is not a solution

## Experiment 11
Linear (not circular) locations

Parameters:
* 10 nodes
* 1980s
* no location mutation
* greedy routing
* direct routing to known neighbor
* circular topology
* reachability averaged over 180s
* ping addresses snooped (ignore swap response, dst on confirmation)
* 1D locations (if distance calc)
* kind swapping

```
    linear distance => 21% 29% 29% 32% 36% avg 29.4%
    log distance    => 39% 31% 43% 25% 37% avg 35.0%
    pow 0.5         => 40% 43% 39% 42% 40% avg 40.8%
    pow 0.1         => 64% 44% 34% 41% 31% avg 42.8%
    pow 0.9         => 42% 31% 34% 42% 17% avg 33.2%
```

## Experiment 12
Goal: check linear locations for fixed randomly generated topology

Parameters:
* 40 nodes
* 1980s
* no location mutation
* greedy routing
* direct routing to known neighbor
* random topology with srand(4) (everything is connected)
* reachability averaged over 180s
* ping addresses snooped (ignore swap response, dst on confirmation)
* 1D locations (linear)
* kind swapping

```
    linear  => 48% 65% 76% 49% 57% avg 59.0%
    pow 0.1 => 73% 43% 45% 49% 62% avg 54.4%
    pow 2   => 44% 39% 52% 31% 41% avg 41.1%
    log     => 42% 52% 40% 50% 45% avg 45.8%
```
    
## Experiment 13
Goal: check circular locations for fixed randomly generated topology. Distances are now actually circular and make sense.

Parameters:
* 40 nodes
* 1980s
* no location mutation
* greedy routing
* direct routing to known neighbor
* random topology with srand(4) (everything is connected)
* reachability averaged over 180s
* ping addresses snooped (ignore swap response, dst on confirmation)
* 1D locations (circular)
* kind swapping

```
    linear     => 55% 43% 49% 58% 50% avg 51.0%
    pow 0.1    => 63% 45% 42% 52% 59% avg 52.2%
    pow 0.5    => 55% 68% 47% 52% 69% avg 58.2%
    pow 2      => 31% 35% 39% 31% 37% avg 34.6%
    log        => 48% 50% 55% 47% 49% avg 49.8%
    pow 0.5 2D => 34% 48% 44% 39% 39% avg 40.8%
    pow 0.5 2Di=> 74% 49% 53% 56% 43% avg 55.0%
    pow 0.5 4Di=> 63% 48% 66% 40% 50% avg 53.4%
    pow 0.5 2Dil=> 47% 51% 51% 52% 54% avg 51.0%
    pow 0.5 2Dl=> 45% 43% 45% 41% 39% avg 42.6%
```

l = 3960 seconds

Conclusion: looking good, linear circular location with ^0.5 is looking best. 1980s seems still enough time for the network to self-organize. If you compare with experiment 3 we have improved from 43% to 58%.

## Night run
2di 0.5 no location mutation 200 nodes 48 hours -> not sure location snooping is sufficient on this scale
at 5940 19%, location mutation seems to be bad

* Took two days
* 17.5%

