/**
Objective function calculation.

The objective function is implemented as a class equipped with its own attributes and methods.
*/

#pragma once

#include <fstream>
#include <iostream>
#include <math.h>
#include <ppl.h>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include "network.hpp"

using namespace std;
using namespace concurrency;

typedef pair<double, int> dist_pair; // used to define a min-priority queue of distance/ID pairs ordered by first element

/**
Objective function class.

A variety of local attributes are used to store information required for calculating the objective function.

Methods are used to execute different steps of the objective function calculation process, much of which is related to distance calculation, and much of which is done in parallel.

This version has been modified to calculate the accessibility metrics of the stop nodes, themselves, rather than the population nodes.
*/
struct Objective
{
	// Public attributes
	Network * Net; // pointer to the main transit network object
	int lowest_metrics = 1; // size of lowest metric set to use for calculating the objective value
	double gravity_exponent = 1.0; // gravity metric distance falloff exponent (will be made negative for calculations)
	double multiplier = 1.0; // multiplication factor for metric values
	int stop_size; // number of stop nodes
	int fac_size; // number of facility nodes

	// Public methods
	Objective(string, Network *); // constructor that reads objective function data and sets network object pointer
	vector<double> all_metrics(); // calculates gravity metrics for all population centers
	void stops_to_all_facilities(int, const vector<double> &, vector<double> &); // calculates distance from a given source stop to all facilities and updates distance vector row
	double facility_metric(int, vector<vector<double>> &); // calculates the gravity metric for a given facility and vector of distances to that facility
	double stop_metric(int, vector<vector<double>> &, vector<double> &); // calculates the gravity metric for a given stop, distance matrix, and facility metric vector
};
