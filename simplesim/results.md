# SimpleSim Results

a   : length of square = a * nodeCount**0.5
dp  : distancePower
c   : connection probability at 1km
nrm : not reachable messages
sp  : shortest path
lp  : location path
f   : average lp/sp

## Influence of the number of nodes

nodes   a       dp  c       wander  nrm     sp      lp      f
10      285     1   0.05    true    true    1.14    1.95    1.1
20      285     1   0.05    true    true    1.75    2.41    1.31
50      285     1   0.05    true    true    2.25    5.79    2.34
100     285     1   0.05    true    true    3.26    17.95   5.43
200     285     1   0.05    true    true    4.38    39.7    8.7
500     285     1   0.05    true    true    6.39    100.33  15.7
1000    285     1   0.05    true    true    8.08    233.8   32.6
2000    285     1   0.05    true    true    11.62   464.7   44.57

We can see that this does not scale. If we assume a factor of 10x slower performance is
acceptable then this would work for a network up to 200 nodes. This is such a small
network that it would be easy for every node to figure out the shortest route
to every node, and we would not need Wanderlust. Only if we can get f to stop
climbing (and keep the same rate of growth as the shortest path) then we have
something that might work.

We can see that problems get severe at 500 nodes, so that is what we will be using
for now. Such a run (using pypy) only takes 0.75 seconds, so fiddling with
parameters (or even running some optimization algorithms over it) will not be problematic.

## Varying the distancePower

nodes   a       dp  c       wander  nrm     sp      lp      f
500     285     0.1 0.05    true    true    6.67    113.3   17.7
500     285     0.5 0.05    true    true    5.86    107.23  17.8
500     285     1   0.05    true    true    6.39    100.33  15.7 (from above)
500     285     1   0.05    true    true    6.03    110.22  19.6
500     285     1   0.05    true    true    6.12    97.95   17.65
500     285     2   0.05    true    true    6.07    97.42   15.3
500     285     3   0.05    true    true    6.68    111.0   15.9
500     285     10  0.05    true    true    6.1     93.5    16.8

As we can see the distancePower doesn't really do much. We'll leave it at 1 for now.

## Improved stats from the simulation

simulated area is 6372.79373587 m by 6372.79373587 m and contains 500 nodes
sent 100 received 100 fraction 1.0
average shortest path 6.37
average location path 96.13
average location path 102.25 (visited)
average fraction 15.41227886
average fraction 16.3436046176 (visited)

This is with backtracking when there are no more nodes to try + keeping track of all visited nodes.

It does not work well to reset the visited nodes to the state when a
node was first reach when it gets a not-reachable message back; messages
will spend ages wandering through the network.

## Max path length

It was also attempted to have a maximum path length, and retry with
increasingly bigger path lenghts if a path could not be found. This did
not work well. The following is with an initial max path length of 15
wich increases to 2x on faillure:

simulated area is 6372.79373587 m by 6372.79373587 m and contains 500 nodes
sent 100 received 100 fraction 1.0
average shortest path 6.6
average location path 34.23
average location path 402.29 (visited)
average fraction 4.85977061827
average fraction 56.0840892996 (visited)

As we can see the length of the paths goes down to 30%, however the
number of nodes visited goes up to 400%! If we note that for
every visited dead end node two packets are sent (one normal and one
"not-reachable") then we can see that the overhead is huge.



