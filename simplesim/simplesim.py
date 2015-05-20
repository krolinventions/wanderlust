# This is a much simpler simulation than the one in the "simulation" directory
# We cheat and take shortcuts in order to provide the basic principle

from __future__ import division

import random
import math

nodeCount = 500
areaSize = 285 * nodeCount**0.5
connection_probability_1km = 0.05
distancePower = 1
wander = False # try to get out of dead ends
not_reachable_messages = True
resetVisitedOnNotReached = False
useMaxLength = False
limitedVisited = False # only store visited if routing AWAY from our destination
dimensions = 200 # dimensions of the location used for location swapping
swapSingleDimension = True
distanceCalculationPower = 1
greedyLocationAssignment = False
locationSwapping = False
locationSmoothing = True
dropDimensions = True

def modularDistance(a, b):
    if a == None or b == None: return 0 # one of the dimensions is not used
    return min([abs(a-b), abs(a-b+1), abs(b-a+1)])

def shortestDistance(a, b):
    result = 0
    for i in xrange(0, len(a)):
        result += modularDistance(a[i],b[i])**distanceCalculationPower
    return result**(1/distanceCalculationPower)

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
        
    def shortestDistanceLocation(self, otherLocation):
        return shortestDistance(self.location, otherLocation)        
        
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

def randomLocation():
    return [random.random() for i in xrange(0, dimensions)]
    
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
        
def bfsPathInfo(source, destination, pathInfo=None):
    if not pathInfo:
        pathInfo = PathInfo()
        pathInfo.source = source
        pathInfo.destination = destination    
    if source == destination:
        pathInfo.route = [destination]
        pathInfo.reached = True
        return pathInfo
    frontier = set([source])
    parent = {}
    def explore(frontier, pathInfo, parent):
        nextFrontier = set()
        for node in frontier:
            if node == destination:
                return 0
            if node in pathInfo.visitedSet: continue
            pathInfo.visitedSet.add(node)
            nextFrontier.update(node.connectedNodes)
            for n in node.connectedNodes:
                if not n in parent: parent[n] = node
        if nextFrontier:
            frontier = set(nextFrontier)
            return explore(frontier, pathInfo, parent)+1
        return no_path;
    result = explore(frontier, pathInfo, parent)
    if result < no_path:
        pathInfo.reached = True
        # the parent map now actually contains the route
        currentNode = destination
        while True:
            pathInfo.route = [currentNode]+pathInfo.route
            if currentNode == source:
                break
            currentNode = parent[currentNode]
    return pathInfo
    
def locationSearch(current, destination, pathInfo=None, maxLength=None, destinationRoutingLocation=None):
    #print "location search current status", source.shortestDistance(destination)
    if not pathInfo:
        pathInfo = PathInfo()
        pathInfo.source = current
        pathInfo.destination = destination
    if not limitedVisited:
        pathInfo.visitedSet.add(current)
    drl = destinationRoutingLocation or destination
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
    
    dlist = [(node.shortestDistanceLocation(drl), node) for node in current.connectedNodes]
    dlist.sort()
    
    for distance, node in dlist:
        if node in pathInfo.visitedSet: # check here as it could be visited by the next locationSearch call
            continue
        if not wander:
            if distance >= current.shortestDistanceLocation(drl):
                # there are no nodes that are closer to the destination and we are not allowed to wander
                return pathInfo
        if limitedVisited and distance >= current.shortestDistanceLocation(drl):
            pathInfo.visitedSet.add(current) # we're going to route away!

        visitedBackup = set(pathInfo.visited)
        # send the packet off to the closest node
        locationSearch(node, destination, pathInfo, maxLength, drl)
        
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
    
def locationSearchDropDimensions(source, destination):
    # search for paths with increasing maximum lengths
    visited = []
    for i in xrange(0,100):
        # drop 10% of the dimensions
        drl = destination.location[:]
        for j in random.sample(xrange(dimensions), int(dimensions*0.5)): drl[j] = None
        pathInfo = locationSearch(source, destination, destinationRoutingLocation=drl)
        visited.extend(pathInfo.visited)
        if pathInfo.reached:
            print "Succeeded after", i+1, "tries"
            # fix up the visited noded
            pathInfo.visited = visited
            return pathInfo
    # not found...
    pathInfo = PathInfo()
    pathInfo.visited = visited
    print "FAILED"
    return pathInfo    
    
# Generate a mesh network
done = False
while not done:
    print "Generating mesh network..."
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

    # now check if all nodes are connected
    print "Checking for isolated nodes..."
    done = True
    firstNode = nodes[0]
    for otherNode in nodes[1:]:
        if bfsPathInfo(firstNode, otherNode).reached == False:
            print "Isolated node found, regenerating";
            done = False
            break

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
    
# possibility: location smoothing
# search the shortest path and "massage" the locations so that
# location routing will have it easier
# this can be done by assuming a linear change in location between
# source and destination and then adjusting the locations of the nodes
# in the shortest path
if locationSmoothing:
    print "Location smoothing..."
    for i in xrange(0, 100000):
        source = random.choice(nodes)
        destination = random.choice(nodes)
        pathInfo = bfsPathInfo(source, destination)
        # gather all locations, sort them and reassign them
        locations = [n.location for n in pathInfo.route]
        sourceLocation = source.location
        destinationLocation = destination.location
        if len(pathInfo.route) > 2:
            for dim in xrange(0, dimensions):
                goUp = (destinationLocation[dim] > sourceLocation[dim] and destinationLocation[dim] - sourceLocation[dim] < 0.5) or (sourceLocation[dim] > destinationLocation[dim] and sourceLocation[dim] - destinationLocation[dim] >= 0.5)
                #print source.location, destination.location, "hops", len(pathInfo.route)-1, "goUp", goUp
                step = modularDistance(sourceLocation[dim], destinationLocation[dim])/(len(pathInfo.route)-1)
                if not goUp: step = -step
                c = (sourceLocation[dim] + step) % 1.0
                for i in xrange(1, len(locations)-1):
                    #print "assigning location", c, "delta", abs(c-pathInfo.route[i].location[dim])
                    pathInfo.route[i].location[dim] = pathInfo.route[i].location[dim] * 0.95 + c * 0.05
                    c = (c + step) % 1.0

# Simulate location swapping
if locationSwapping:
    print "Location swapping..."
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
    if useMaxLength: 
        pathInfo = locationSearchMaxLength(source, destination)
    elif dropDimensions:
        pathInfo = locationSearchDropDimensions(source,destination)
    else:
        pathInfo = locationSearch(source,destination)
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
