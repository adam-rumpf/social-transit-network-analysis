/**
Main driver for network analysis tools.

This project includes submodules for analyzing various aspects of the input netowrk, including the following:
-Evaluating whether the initial flow vector is "reasonable" by examining the flow:capacity ratio of each arc.
-Evaluating the accessibility metric of every stop (using the population accessibility metric module, but treating the stops as population centers).
-Selecting and constructing express route candidates.
*/

#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
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

// Define output file names
#define STOP_METRIC_FILE "output/stop_metrics.txt"
#define LINE_METRIC_FILE "output/line_metrics.txt"

using namespace std;

typedef pair<double, int> stop_pair; // used to define a min-priority queue of metric/stop ID pairs

// Function prototypes
void loading_factors(Network *);
void record_stop_metrics(Network *, const vector<double> &);
void record_line_metrics(Network *, const vector<double> &);

/// Main driver.
int main()
{
	// Initialize network object
	Network * Net = new Network(NODE_FILE, ARC_FILE, TRANSIT_FILE, VEHICLE_FILE, PROBLEM_FILE, FLOW_FILE);

	// Display example alpha/beta parameter pairs
	cout << "Example alpha/beta pairs:" << endl;
	for (double alpha = 2.0; alpha <= 24.0; alpha += 2.0)
		cout << "(" << alpha << ", " << (2 * alpha - 1) / (2 * alpha - 2) << ")" << setprecision(15) << endl;
	cout << endl;

	// Calculate loading factor statistics
	loading_factors(Net);

	// Initialize objective object
	Objective * Obj = new Objective(OBJECTIVE_FILE, Net);

	// Calculate accessibility metrics of stops
	vector<double> stop_metrics = Obj->all_metrics();

	// Output stop metric file
	record_stop_metrics(Net, stop_metrics);

	// Output delineated stop metric file
	record_line_metrics(Net, stop_metrics);

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
	cout << "Average loading factor (line arcs only): " << tot / Net->line_arcs.size() << endl << endl;
}

/// Outputs the stop-level accessibility metrics.
void record_stop_metrics(Network * net_in, const vector<double> &metrics)
{
	Network * Net = net_in;

	ofstream out_file(STOP_METRIC_FILE);

	if (out_file.is_open())
	{
		cout << "Writing metrics to output file..." << endl;

		// Write comment line
		out_file << "Stop_ID\tGravity_Metric" << fixed << setprecision(15) << endl;

		// Write all metrics
		for (int i = 0; i < metrics.size(); i++)
			out_file << Net->stop_nodes[i]->id << '\t' << metrics[i] << endl;

		out_file.close();
		cout << "Successfuly recorded metrics!" << endl;
	}
	else
		cout << "Metric file failed to open." << endl;
}

/// Outputs the stop-level accessibility metrics, divided by line and sorted in ascending order for each line.
void record_line_metrics(Network * net_in, const vector<double> &metrics)
{
	Network * Net = net_in;

	/*
	We store the line-level stop metrics as a vector of prority queues.

	There is one priority queue for each line, containing metric/stop ID pairs in ascending order of metric.
	*/

	// Build a map from stop node IDs to metric vector indices
	unordered_map<int, int> stop_remap;
	for (int i = 0; i < Net->stop_nodes.size(); i++)
		stop_remap[Net->stop_nodes[i]->id] = i;

	// Build the line stop lists
	vector<priority_queue<stop_pair, vector<stop_pair>, greater<stop_pair>>> line_stops(Net->lines.size());
	for (int i = 0; i < Net->lines.size(); i++)
	{
		for (int j = 0; j < Net->lines[i]->stops.size(); j++)
		{
			int nid = Net->lines[i]->stops[j]->id;
			double met = metrics[stop_remap[nid]];
			line_stops[i].push(make_pair(met, nid));
		}
	}

	ofstream out_file(LINE_METRIC_FILE);

	if (out_file.is_open())
	{
		cout << "Writing line metrics to output file..." << endl;

		// Write comment line
		out_file << "Line_ID\tStop_ID\tGravity_Metric" << fixed << setprecision(15) << endl;

		// Write all metrics
		for (int i = 0; i < line_stops.size(); i++)
		{
			while (line_stops[i].empty() == false)
			{
				out_file << i << '\t' << line_stops[i].top().second << '\t' << line_stops[i].top().first << endl;
				line_stops[i].pop();
			}
		}

		out_file.close();
		cout << "Successfuly recorded line metrics!" << endl;
	}
	else
		cout << "Line metric file failed to open." << endl;
}
