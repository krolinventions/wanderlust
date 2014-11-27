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
resetVisitedOnNotReached = False
useMaxLength = False
limitedVisited = False # only store visited if routing AWAY from our destination
dimensions = 1 # dimensions of the location used for location swapping
swapSingleDimension = False
manhattan = False # should distance calculations for multiple dimensions use manhattan distance?
greedyLocationAssignment = True

def shortestDistance(a, b):
    result = 0
    if manhattan: p = 1
    else: p = 2
    for i in xrange(0, len(a)):
        result += min([abs(a[i]-b[i]), abs(a[i]-b[i]+1), abs(b[i]-a[i]+1)])**p
    return result**(1/p)

class Node(object):
    x = 0
    y = 0
    connectedNodes = None
    
    def __init__(self):
        self.connectedNodes = []
        self.location = None
    
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
        
    def swap(self, other, d):
        if swapSingleDimension:
            self.location[d], other.location[d] = other.location[d], self.location[d]
        else:
            self.location, other.location = other.location, self.location
        
    def shouldSwap(self, other, d):
        scoreBefore = self.locationScore() + other.locationScore()
        self.swap(other, d)
        scoreAfter = self.locationScore() + other.locationScore()
        self.swap(other, d)
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

def randomLocation():
    return [random.random() for i in xrange(0, dimensions)]

if greedyLocationAssignment:
    # create a list of random locations
    randoms = [randomLocation() for n in nodes]
    todo = set(nodes)
    while todo:
        # first one
        n = todo.pop()
        n.location = randoms.pop() # give it a location from our list
        frontier = set(n.connectedNodes)
        for node in frontier:
            if node.location: continue
            # find the closest random location
            s = [(shortestDistance(n.location, x), x) for x in randoms]
            s.sort()
            node.location = s[0][1]
            randoms.remove(node.location)
            todo.remove(node)
else:
    # Assign everyone an initial location
    for node in nodes: node.location = randomLocation()

# Simulate location swapping
if 1:
    totalScore = 0
    for d in xrange(0, dimensions):
        for i in xrange(0, 1000):
            swapCount = 0
            for node in nodes:
                for other in node.connectedNodes:
                    if node.shouldSwap(other, d):
                        node.swap(other, d)
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
    
class PathInfo(object):
    def __init__(self):
        self.route = [] # first hop is first
        self.source = None
        self.destination = None
        self.visited = []
        self.reached = False
        self.visitedSet = set()
    
def locationSearch(current, destination, pathInfo=None, maxLength=None):
    #print "location search current status", source.shortestDistance(destination)
    if not pathInfo:
        pathInfo = PathInfo()
        pathInfo.source = current
        pathInfo.destination = destination
    if not limitedVisited:
        pathInfo.visitedSet.add(current)
    pathInfo.route.append(current)
    pathInfo.visited.append(current)
    if current == destination:
        pathInfo.reached = True
        return pathInfo
    if maxLength and len(pathInfo.route) > maxLength:
        return pathInfo # maximum length of route reached
    
    # we assume we know the address of our connected nodes
    if destination in current.connectedNodes:
        pathInfo.route.append(destination)
        pathInfo.visited.append(destination)
        pathInfo.reached = True
        return pathInfo
    
    dlist = [(node.shortestDistance(destination), node) for node in current.connectedNodes]
    dlist.sort()
    
    for distance, node in dlist:
        if node in pathInfo.visitedSet: # check here as it could be visited by the next locationSearch call
            continue
        if not wander:
            if distance >= current.shortestDistance(destination):
                # there are no nodes that are closer to the destination and we are not allowed to wander
                return pathInfo
        if limitedVisited and distance >= current.shortestDistance(destination):
            pathInfo.visitedSet.add(current) # we're going to route away!

        visitedBackup = set(pathInfo.visited)
        # send the packet off to the closest node
        locationSearch(node, destination, pathInfo, maxLength)
        
        if pathInfo.reached:
            # we've reached our destination
            return pathInfo
        if not_reachable_messages:
            # if we have not reachable messages we can retry with another one
            # remove the last node from the route (the one that didn't work out)
            pathInfo.route.pop()
            if resetVisitedOnNotReached:
                # also reset the visited nodes as we want to start off with a clean slate from here on
                pathInfo.visited = visitedBackup
            continue
            
        # sadly we could not reach our destination
        return pathInfo
    # we ran out of nodes to send to
    return pathInfo
    
def locationSearchMaxLength(source, destination):
    # search for paths with increasing maximum lengths
    maxLength = 15
    visited = []
    previousVisited = 0
    while True:
        pathInfo = locationSearch(source, destination, maxLength=maxLength)
        visited.extend(pathInfo.visited)
        if pathInfo.reached:
            # fix up the visited noded
            pathInfo.visited = visited
            return pathInfo
        if len(pathInfo.visited) == previousVisited:
            break # no progress made
        previousVisited = len(pathInfo.visited)
        maxLength *= 2
    # not found...
    print "giving up at maxLength", maxLength
    pathInfo = PathInfo()
    pathInfo.visited = visited
    return pathInfo

# See if we can route packets
to_send = 100
sent = 0
received = 0
shortestPaths = []
locationPaths = []
locationPathsVisited = []
lpVisitedSets = []
fractions = []
fractionsNoWaste = []
for i in xrange(0, to_send):
    source = random.choice(nodes)
    destination = random.choice(nodes)
    path = bfs(source, destination)
    if path >= no_path: continue
    
    sent += 1
    # see if we can find the route by going to the most alike location
    if not useMaxLength: 
        pathInfo = locationSearch(source,destination)
    else:
        pathInfo = locationSearchMaxLength(source, destination)
    print "shortest", path, "location routing", pathInfo.reached, len(pathInfo.route)-1, "visited", len(pathInfo.visited)-1
    if pathInfo.reached:
        received += 1
        shortestPaths.append(path)
        locationPaths.append(len(pathInfo.route)-1)
        locationPathsVisited.append(len(pathInfo.visited)-1)
        lpVisitedSets.append(len(pathInfo.visitedSet))
        if path > 0:
            fractions.append((len(pathInfo.route)-1)/path)
            fractionsNoWaste.append((len(pathInfo.visited)-1)/path)
print "=============================="
print "Results:"

def avg(x): return sum(x)/len(x)

print "simulated area is", areaSize, "m by", areaSize, "m and contains", nodeCount, "nodes"
print "sent", sent, "received", received, "fraction", received/sent
print "average shortest path", sum(shortestPaths)/len(shortestPaths)
print "average location path", sum(locationPaths)/len(locationPaths)
print "average location path", sum(locationPathsVisited)/len(locationPathsVisited), "(visited)"
print "average location path", avg(lpVisitedSets), "(rembered visited)"
print "average fraction", sum(fractions)/len(fractions)
print "average fraction", sum(fractionsNoWaste)/len(fractionsNoWaste), "(visited)"
