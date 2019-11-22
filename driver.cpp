/**
Main driver for network analysis tools.

This project includes submodules for analyzing various aspects of the input netowrk, including the following:
-Evaluating whether the initial flow vector is "reasonable" by examining the flow:capacity ratio of each arc.
-Evaluating the accessibility metric of every stop (using the population accessibility metric module, but treating the stops as population centers).
-Selecting and constructing express route candidates.
*/

#include <iomanip>
#include <iostream>
#include <vector>
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

// Function prototypes
void loading_factors(Network *);

/// Main driver.
int main()
{
	// Initialize network object
	Network * Net = new Network(NODE_FILE, ARC_FILE, TRANSIT_FILE, VEHICLE_FILE, PROBLEM_FILE, FLOW_FILE);

	// Display example alpha/beta parameter pairs
	cout << "Example alpha/beta pairs:" << endl;
	for (double alpha = 2.0; alpha <= 12.0; alpha += 1.0)
		cout << "(" << alpha << ", " << (2 * alpha - 1) / (2 * alpha - 2) << ")" << setprecision(15) << endl;
	cout << endl;

	// Calculate loading factor statistics
	loading_factors(Net);

	cin.get();

	return 0;
}

/*
Calculates the loading factors for the arcs in a given network.

Requires a reference to a network object.

Calculates the loading factors of all core arcs and prints various statistics to the screen.
*/
void loading_factors(Network * net_in)
{
	Network * Net = net_in;

	// Calculate all loading factors
	vector<double> factors(Net->core_arcs.size(), 0.0);
	for (int i = 0; i < Net->core_arcs.size(); i++)
	{
		Arc * a = Net->core_arcs[i];
		if (a->line >= 0)
			factors[i] = a->flow / Net->lines[a->line]->capacity();
	}

	// Calculate maximum arc loading factor
	double max_load = 0;
	for (int i = 0; i < factors.size(); i++)
		max_load = max(factors[i], max_load);
	cout << "Maximum loading factor: " << max_load << endl;

	// Calculate average loading factors
	double tot = 0;
	for (int i = 0; i < factors.size(); i++)
		tot += factors[i];
	cout << "Average loading factor (all core arcs):  " << tot / Net->core_arcs.size() << endl;
	cout << "Average loading factor (line arcs only): " << tot / Net->line_arcs.size() << endl;
}
