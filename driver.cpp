/**
Main driver for network analysis tools.

This project includes submodules for analyzing various aspects of the input netowrk, including the following:
-Evaluating whether the initial flow vector is "reasonable" by examining the flow:capacity ratio of each arc.
-Evaluating the accessibility metric of every stop (using the population accessibility metric module, but treating the stops as population centers).
-Selecting and constructing express route candidates.
*/

#include <iostream>
#include "network.hpp"
#include "objective.hpp"

// Define input file names
#define NODE_FILE "data/node_data.txt"
#define ARC_FILE "data/arc_data.txt"
#define TRANSIT_FILE "data/transit_data.txt"
#define VEHICLE_FILE "data/vehicle_data.txt"
#define OBJECTIVE_FILE "data/objective_data.txt"
#define PROBLEM_FILE "data/problem_data.txt"
#define FLOW_FILE "data/initial_flows.txt"

using namespace std;

/// Main driver.
int main()
{
	// Initialize network object
	Network * Net = new Network(NODE_FILE, ARC_FILE, TRANSIT_FILE, VEHICLE_FILE, PROBLEM_FILE, FLOW_FILE);

	cin.get();

	return 0;
}
