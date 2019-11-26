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
#define FINAL_SOLUTION_FILE "data/final.txt"
#define SOLUTION_LOG_FILE "data/solution.txt"
#define INITIAL_SOLUTION_LOG_FILE "data/initial_solution_log.txt"

// Define output file names
#define STOP_METRIC_FILE "output/stop_metrics.txt"
#define LINE_METRIC_FILE "output/line_metrics.txt"

using namespace std;

typedef pair<double, int> stop_pair; // used to define a min-priority queue of metric/stop ID pairs

// Function prototypes
void loading_factors(Network *);
void record_stop_metrics(Network *, const vector<double> &);
void record_line_metrics(Network *, const vector<double> &);
void solution_log_stats();
void compare_solutions(Network *);
vector<int> str2vec(string, char);

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

	// Calculate solution log statistics
	solution_log_stats();

	// Calculate solution comparison
	compare_solutions(Net);

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

	// Count factors within certain ranges
	vector<double> bounds = { -1, 0.0, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0, INFINITY };
	vector<int> counts(bounds.size(), 0);
	for (int i = 0; i < factors.size(); i++)
	{
		int j = 0;
		while (factors[i] > bounds[j])
			j++;
		counts[j - 1]++;
	}
	cout << "Number of core arcs in each range:" << endl;
	for (int i = 0; i < bounds.size() - 1; i++)
	{
		if (i == 0)
			cout << "[0, ";
		else
			cout << "(" << bounds[i] << ", ";
		cout << bounds[i + 1];
		if (i == bounds.size())
			cout << "inf) : ";
		else
			cout << "] : ";
		cout << counts[i] << endl;
	}
	cout << endl;

	// Output arcs with an excessive load factor
	cout << "Arcs with load factors of more than 1.5:" << endl;
	for (int i = 0; i < factors.size(); i++)
	{
		if (factors[i] > 1.5)
		{
			Arc * a = Net->core_arcs[i];
			cout << "Arc " << a->id << " (" << a->tail->id << ", " << a->head->id << "), Load " << factors[i] << ", Line " << a->line << endl;
		}
	}
	cout << endl;
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

/// Calculates solution log statistics.
void solution_log_stats()
{
	vector<int> feasible_count = { 0, 0, 0 }; // counts of unknown/infeasible/feasible results

	// Read solution log
	cout << "Reading solution log..." << endl;
	ifstream sol_file;
	sol_file.open(SOLUTION_LOG_FILE);
	if (sol_file.is_open())
	{
		string line, piece; // whole line and line element being read
		getline(sol_file, line); // skip comment line
		int count = 0;

		while (sol_file.eof() == false)
		{
			// Get whole line as a string stream
			getline(sol_file, line);
			if (line.size() == 0)
				// Break for blank line at file end
				break;
			stringstream stream(line);

			// Go through each piece of the line
			getline(stream, piece, '\t'); // solution
			getline(stream, piece, '\t'); // feasible
			int feas = stoi(piece);

			// Tally feasibility
			switch (feas)
			{
			case -1:
				feasible_count[0]++;
				break;
			case 0:
				feasible_count[1]++;
				break;
			case 1:
				feasible_count[2]++;
				break;
			}
		}

		sol_file.close();
		cout << "Solution log read!" << endl;
	}
	else
		cout << "Solution log failed to open." << endl;

	cout << "\n\nSolution log statistics (all):" << fixed << setprecision(2) << endl;
	int feasible_total = feasible_count[0] + feasible_count[1] + feasible_count[2];
	cout << "Total:              " << feasible_total << endl;
	cout << "Percent unknown:    " << (100.0 * feasible_count[0]) / feasible_total << " %" << endl;
	cout << "Percent infeasible: " << (100.0 * feasible_count[1]) / feasible_total << " %" << endl;
	cout << "Percent feasible:   " << (100.0 * feasible_count[2]) / feasible_total << " %" << endl;
	cout << "\nSolution log statistics (known only):" << endl;
	feasible_total = feasible_count[1] + feasible_count[2];
	cout << "Total:              " << feasible_total << endl;
	cout << "Percent infeasible: " << (100.0 * feasible_count[1]) / feasible_total << " %" << endl;
	cout << "Percent feasible:   " << (100.0 * feasible_count[2]) / feasible_total << " %" << endl << endl;
}

/// Compares initial solution to final solution.
void compare_solutions(Network * net_in)
{
	Network * Net = net_in;

	string sol_string = "";

	// Read initial solution log
	cout << "Reading initial solution log..." << endl;
	ifstream sol_file;
	sol_file.open(SOLUTION_LOG_FILE);
	if (sol_file.is_open())
	{
		string line, piece; // whole line and line element being read
		getline(sol_file, line); // skip comment line
		int count = 0;

		while (sol_file.eof() == false)
		{
			// Get whole line as a string stream
			getline(sol_file, line);
			if (line.size() == 0)
				// Break for blank line at file end
				break;
			stringstream stream(line);

			// Go through each piece of the line
			getline(stream, piece, '\t'); // solution
			sol_string = piece;
		}

		sol_file.close();
		cout << "Solution log read!" << endl;
	}
	else
		cout << "Solution log failed to open." << endl;

	// Convert solution string to a solution vector
	vector<int> sol_initial = str2vec(sol_string, '_');

	// Read final solution
	cout << "Reading final solution..." << endl;
	sol_file.open(FINAL_SOLUTION_FILE);
	if (sol_file.is_open())
	{
		string line, piece; // whole line and line element being read
		getline(sol_file, line);
		sol_string = line;

		sol_file.close();
		cout << "Solution read!" << endl;
	}
	else
		cout << "Solution failed to open." << endl;

	// Convert solution string to a solution vector
	vector<int> sol_final = str2vec(sol_string, '\t');

	// Calculate changes in solution vector elements
	priority_queue<tuple<int, int, int, string>> comparison_queue;
	for (int i = 0; i < sol_initial.size(); i++)
		comparison_queue.push(make_tuple(abs(sol_final[i] - sol_initial[i]), sol_final[i] - sol_initial[i], i, Net->lines[i]->name));
	cout << "\nSolution element changes (sorted by amount of change):" << endl;
	cout << "ID\tName\tChange" << endl;
	while (comparison_queue.empty() == false)
	{
		int diff = get<1>(comparison_queue.top());
		int id = get<2>(comparison_queue.top());
		string name = get<3>(comparison_queue.top());
		string diff_string;
		if (diff > 0)
			diff_string = '+' + to_string(diff);
		else
			diff_string = to_string(diff);
		comparison_queue.pop();
		cout << id << '\t' << name << '\t' << diff_string << endl;
	}
	cout << endl;
}

/// Converts a solution string back into an integer solution vector.
vector<int> str2vec(string sol, char delimiter)
{
	vector<int> out;
	stringstream sol_stream(sol);
	string element;

	while (getline(sol_stream, element, delimiter))
		out.push_back(stoi(element));

	return out;
}
