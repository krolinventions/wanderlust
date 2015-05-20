# SimpleSim Results

```
a   : length of square = a * nodeCount**0.5
dp  : distancePower
c   : connection probability at 1km
nrm : not reachable messages
sp  : shortest path
lp  : location path
f   : average lp/sp
r   : reachability fraction
```

## Influence of the number of nodes

```
nodes   a       dp  c       wander  nrm     sp      lp      f
10      285     1   0.05    true    true    1.14    1.95    1.1
20      285     1   0.05    true    true    1.75    2.41    1.31
50      285     1   0.05    true    true    2.25    5.79    2.34
100     285     1   0.05    true    true    3.26    17.95   5.43
200     285     1   0.05    true    true    4.38    39.7    8.7
500     285     1   0.05    true    true    6.39    100.33  15.7
1000    285     1   0.05    true    true    8.08    233.8   32.6
2000    285     1   0.05    true    true    11.62   464.7   44.57
```

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

```
nodes   a       dp  c       wander  nrm     sp      lp      f
500     285     0.1 0.05    true    true    6.67    113.3   17.7
500     285     0.5 0.05    true    true    5.86    107.23  17.8
500     285     1   0.05    true    true    6.39    100.33  15.7 (from above)
500     285     1   0.05    true    true    6.03    110.22  19.6
500     285     1   0.05    true    true    6.12    97.95   17.65
500     285     2   0.05    true    true    6.07    97.42   15.3
500     285     3   0.05    true    true    6.68    111.0   15.9
500     285     10  0.05    true    true    6.1     93.5    16.8
```

As we can see the distancePower doesn't really do much. We'll leave it at 1 for now.

## Improved stats from the simulation

```
simulated area is 6372.79373587 m by 6372.79373587 m and contains 500 nodes
sent 100 received 100 fraction 1.0
average shortest path 6.37
average location path 96.13
average location path 102.25 (visited)
average fraction 15.41227886
average fraction 16.3436046176 (visited)
```

This is with backtracking when there are no more nodes to try + keeping track of all visited nodes.

It does not work well to reset the visited nodes to the state when a
node was first reach when it gets a not-reachable message back; messages
will spend ages wandering through the network.

## Max path length

It was also attempted to have a maximum path length, and retry with
increasingly bigger path lenghts if a path could not be found. This did
not work well. The following is with an initial max path length of 15
wich increases to 2x on faillure:

```
simulated area is 6372.79373587 m by 6372.79373587 m and contains 500 nodes
sent 100 received 100 fraction 1.0
average shortest path 6.6
average location path 34.23
average location path 402.29 (visited)
average fraction 4.85977061827
average fraction 56.0840892996 (visited)
```

As we can see the length of the paths goes down to 30%, however the
number of nodes visited goes up to 400%! If we note that for
every visited dead end node two packets are sent (one normal and one
"not-reachable") then we can see that the overhead is huge.

## Limiting the number of remembered visited nodes

Normal run:
```
simulated area is 6372.79373587 m by 6372.79373587 m and contains 500 nodes
sent 100 received 100 fraction 1.0
average shortest path 6.15
average location path 108.38
average location path 119.27 (visited)
average location path 119.27 (rembered visited)
average fraction 18.2292262737
average fraction 19.9483864469 (visited)
```

Only remember when routing away from the destination:
```
simulated area is 6372.79373587 m by 6372.79373587 m and contains 500 nodes
sent 100 received 100 fraction 1.0
average shortest path 6.36
average location path 137.13
average location path 146.9 (visited)
average location path 96.26 (rembered visited)
average fraction 21.5059603976
average fraction 23.0472524669 (visited)
```

The average path length goes up, and the number of remembered visited
nodes does not significantly go down. Doesn't seem to have
much impact at the current state of the algorithm.

## Multiple dimensions

We swap the entire location at once, and we use manhattan distance.
```
dim f
1   17.22
1   15.7
2   14.22
2   19.8
3   12.7
3   14.2
4   15.55
5   15.6
```

This does not seem to make any make any difference. Now let's try this without manhattan distance.

```
d   f
1   21.3
1   17.0
2   14.02
2   16.98
3   15.74
3   16.7
4   13.7
4   13.5
4   12.7
4   19.01
5   15.25
5   16.7
```

It does seem that the variability of the f factor is too high to say anything
for this limited number of runs.

We now try location swapping the individual dimensions (dimensions in the outer loop)

```
d   f
4   13.3
4   15.4
4   15.7
```

This does not really seem to make a difference. How about we swap the dimensions in the
inner loop?

```
d   f
4   18.18
4   18.47
4   15.9
4   19.3
```

Well, this actually seems to have made it worse...

## Improving location swapping

Maybe the location swapping algorithm is not good enough. Let's try with something more advanced.

1 dimension
```
f
14.11
14.86
13.13
16.11
13.5
18.4
```

That did not seem to work as well as hoped. Let's try to assign the initial locations even better.

## Location smoothing
Instead of using the location swapping algorithm to assign the locations we use a global algorithm.
Two nodes are chosen and the shortest path between them is calculated using bfs. Then all the
nodes in between get assigned new locations so that when using location routing this will be a favorable
route. Instead of fully assigning the location to the calculated optimal one it is only adjusted 5%.
No location swapping is used. Distancepower is 1. Locationsmoothing is run for 10000 iterations.

```
f
13.0
12.11
11.8
11.18
11.58
````

We can see that this leads to a significant improvement. How about we use more dimensions?

```
d   f
2   10.44
2   9.77
3   6.49
3   8.95
4   8.75
4   8.41
5   8.41
5   6.22
10  5.1
10  5.5
20  5.17
20  4.28
40  3.90
40  2.90
100 2.66
100 2.29
200 2.92
200 2.17
```

Adding more dimensions really does work this time. We might want to use a different distance function,
maybe one that takes all bits into account separately. XOR distance might be a good solution.
We might even just count the number of different bits.
It is interesting to repeat the experiment and disable wandering to see if reachability has improved:

```
d   f       r
1   1.36    0.43
2   1.25    0.46
3   1.16    0.39
4   1.24    0.53
5   1.17    0.31
10  1.21    0.61
20  1.20    0.62
40  1.18    0.64
100 1.22    0.73
200 1.18    0.81
400 1.18    0.69
1000 1.22   0.81
2000 1.11   0.91
4000 1.15   0.84
```

We can see that with adding more dimensions reachability does go up.

## Dropping some dimensions when routing

It is possible that not all dimensions are of help when routing to a certain node. We could try sending
packets with some with some dimensions unspecified. Those should be skipped when routing. The way this was
implemented is that we try max 10 times with each time <ign> of all dimensions ignored.

```
ign     r
0       0.79
0.1     0.92
0.1     0.90
0.1     0.82
0.5     0.98
0.5     0.95
0.5     0.95
0.9     0.95
0.9     0.95
0.9     0.89
0.99    0.88
0.99    0.97
0.99    0.87
```

We can see that this works extremely well! For now we will drop 50% of our dimensions (currently 200 in total).

What would happen if we increased the retries <ret??

```
ret     r       f(visited)
1       0.84    2.55
2       0.76    3.11
4       0.95    2.7
10      0.98    4.9
20      0.98    5.3
40      0.98    5.6
100     1.0     7.7
200     1.0     3.4
400     0.99    6.9
1000    0.99    5.6
2000    1.0     15.18
```

Increasing the number of retries allows us to reach 100% reachability in most cases.
The total overhead goes up when the number of retries increases. 10 seems a good value for the
simulation.

## Swapping again

Now, how does this work with location swapping? Let's try swapping individual dimensions as we want them to
be as independent as possible. Location swapping 200 dimensions is currently way to slow so we are
easing our way up.

```
d   r
1   0.21
2   0.27
3   0.32
4   0.26
10  0.48
20  0.45
```

Location swapping doesn't seem to improve at >10 dimensions. What happens when we use
location smoothing?

```
d   r
1   0.39
2   0.51
4   0.77
10  0.9
20  0.93
40  0.83
100 0.92
200 0.93
200 0.99 (dropretries naar 100)
200 0.99 (dropretries naar 100)
```

It still seems that to get to 99% reachability you need > 10 retries.

Location swapping doesn't seem to work well in this case. We might find a practical
implementation of location smoothing in the future (which can be implemented in a robust
way with real packets).

## Location smoothing scalability

Tries is 100.

```
n       r       f(visited)
500     0.98    9.2
1000    0.88    20.7
2000    0.39    55.49
```

We can see this is still not scalable. Let's increase the number of iterations for location
smoothing from 10000 to 100000.

```
n       r       f (visited)
500     0.99    1.85
1000    1.0     4.33
2000    0.96    41
```

This actually seems to work great. We might need more location swapping rounds for the locations to converge.
