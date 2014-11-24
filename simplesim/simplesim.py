# This is a much simpler simulation than the one in the "simulation" directory
# We cheat and take shortcuts in order to provide the basic principle

from __future__ import division

import random
import math

nodeCount = 500
areaSize = 285 * nodeCount**0.5
connection_probability_1km = 0.05
distancePower = 1
wander = True # try to get out of dead ends
not_reachable_messages = True

def shortestDistance(a, b):
    return min([abs(a-b), abs(a-b+1), abs(b-a+1)])

class Node:
    x = 0
    y = 0
    connectedNodes = None
    location = 0
    
    def __init__(self):
        self.connectedNodes = []
    
    def distanceSquared(self, other):
        return (self.x-other.x)**2 + (self.y-other.y)**2
        
    def shortestDistance(self, other):
        #print self.location, other.location, shortestDistance(self.location, other.location), shortestDistance(other.location, self.location)
        #assert(shortestDistance(self.location, other.location) == shortestDistance(other.location, self.location))
        return shortestDistance(self.location, other.location)
        
    def locationScore(self):
        score = 0
        for other in self.connectedNodes:
            score += self.shortestDistance(other)**distancePower
        return score
        
    def swap(self, other):
        self.location, other.location = other.location, self.location
        
    def shouldSwap(self, other):
        scoreBefore = self.locationScore() + other.locationScore()
        self.swap(other)
        scoreAfter = self.locationScore() + other.locationScore()
        self.swap(other)
        return scoreAfter < scoreBefore

# Generate a mesh network
nodes = []
for i in xrange(0, nodeCount):
    node = Node()
    node.x = random.random()*areaSize
    node.y = random.random()*areaSize
    nodes.append(node)
    
for node1 in nodes:
    for node2 in nodes:
        if node1 == node2: continue
        distanceSquared = node1.distanceSquared(node2)
        # 0m -> 100%, 1000m -> 5%
        probablility = math.exp(-5E-6*distanceSquared)
        # NS_LOG_INFO ("Node " << i << " and " << j << " probability " << probablilty);
        if random.random() < probablility/5*connection_probability_1km*100:
            node1.connectedNodes.append(node2)
            node2.connectedNodes.append(node1)
            
# Assign everyone an initial location
for node in nodes: node.location = random.random()

# Simulate location swapping
if 1:
    totalScore = 0
    for i in xrange(0, 1000):
        swapCount = 0
        for node in nodes:
            for other in node.connectedNodes:
                if node.shouldSwap(other):
                    node.swap(other)
                    swapCount += 1
        oldScore = totalScore
        totalScore = sum([node.locationScore() for node in nodes])
        print "Location swap round", i, swapCount, "swaps", totalScore, "total score"
        if swapCount == 0: break
        if oldScore == totalScore: break

# bfs for the shortest route (in hops)
def dfs(s, d):
    visited = set()

    def dfsInner(source, destination):
        if source == destination: return 0
        visited.add(source)
        result = min([dfsInner(node, destination) for node in source.connectedNodes if node not in visited]) + 1
        visited.remove(source)
        return result

    return dfsInner(s, d)
    
no_path = 1000000
def bfs(source, destination):
    global frontier
    if source == destination: return 0
    visited = set()
    frontier = set([source])
    def explore():
        global frontier
        #print frontier, visited
        nextFrontier = set()
        for node in frontier:
            if node == destination:
                return 0
            if node in visited: continue
            visited.add(node)
            nextFrontier.update(node.connectedNodes)
            #print "adding node to visited", len(visited)
        if nextFrontier:
            frontier = set(nextFrontier)
            return explore()+1
        return no_path;
    result = explore()
    #print "bfs visited", len(visited), "nodes"
    return result
    
def locationSearch(current, destination, visited=None):
    #print "location search current status", source.shortestDistance(destination)
    if current == destination: return 0
    if not visited: visited = set()
    visited.add(current)
    
    if destination in current.connectedNodes: return 1 # we assume we know the address of our connected nodes
    
    dlist = [(node.shortestDistance(destination), node) for node in current.connectedNodes if node not in visited]
    dlist.sort()
    if not dlist: return no_path
    
    waste = 0 # the waste of the not rechable messages
    for distance, node in dlist:
        if not wander:
            if distance >= current.shortestDistance(destination): return no_path # dead end!

        # send the packet off to the closest node
        lsearchresult = locationSearch(node, destination, visited)
        if lsearchresult < no_path:
            return lsearchresult + 1 + waste
        if not_reachable_messages == False:
            return lsearchresult + 1 + waste # we have to accept this result
        # if we have not reachable messages we can retry with another one
        # update the waste
        waste += (lsearchresult-no_path)*2
    return no_path

# See if we can route packets
to_send = 100
sent = 0
received = 0
shortestPaths = []
locationPaths = []
fractions = []
for i in xrange(0, to_send):
    source = random.choice(nodes)
    destination = random.choice(nodes)
    path = bfs(source, destination)
    if path >= no_path: continue
    
    sent += 1
    # see if we can find the route by going to the most alike location
    lpath = locationSearch(source, destination)
    print "shortest", path, "location routing", lpath
    if lpath < no_path:
        received += 1
        shortestPaths.append(path)
        locationPaths.append(lpath)
        if path > 0:
            fractions.append(lpath/path)
print "=============================="
print "Results:"

print "simulated area is", areaSize, "m by", areaSize, "m and contains", nodeCount, "nodes"
print "sent", sent, "received", received, "fraction", received/sent
print "average shortest path", sum(shortestPaths)/len(shortestPaths)
print "average location path", sum(locationPaths)/len(locationPaths)
print "average fraction", sum(fractions)/len(fractions)
