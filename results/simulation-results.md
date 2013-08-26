# Simulation Results

This page currently is a dumping ground for notes taken during simulations.

## Experiment 1
Goal: check difference between 1D and 2D for a larger amount of nodes and a bigger amount of time.

Parameters:
* 200 nodes
* 80 hours
* location mutation rand()%0x02000000 - 0x01000000 every 60-120 seconds
* Direct routing to known neighbor

Runs:
* Locations 2D => ~15% reachability
* Locations 1D => ~16% reachability

Conclusion: no significant difference. Maybe when local routing is added this will change.

