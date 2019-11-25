# social-transit-network-analysis

Preprocessing tools for analyzing the input network for a research project of mine dealing with a public transit design model with social access objectives. This is meant for use in analyzing the network after preprocessing with [social-transit-solver-single](https://github.com/adam-rumpf/social-transit-solver-single), with which this repository shares a large amount of code.

Specifically, this is a set of programs meant for performing the following tasks:

* [Loading Factor Calculation (C++)](#loading-factor-calculation)
* [Stop-Level Metrics (C++)](#stop-level-metrics)
* [Candidate Express Route Generation (Mathematica)](#candidate-express-route-generation)

These procedures are explained in more detail below. They are so specific to my applications that I would not expect them to be useful to anyone outside of my research group, but they are being provided here for anyone interested.

## Data Folder

This program reads input files from a local `data/` folder. The following data files should be included in this folder:

* `arc_data.txt`
* `gravity_metrics.txt`
* `initial_flows.txt`
* `initial_solution_log.txt`
* `node_data.txt`
* `objective_data.txt`
* `problem_data.txt`
* `transit_data.txt`
* `user_cost_data.txt`
* `vehicle_data.txt`

Most of these correspond to the standard network input files used by the other programs that are part of this project, except for `gravity_metrics.txt` and `initial_flows.txt`, which are produced by the preprocessor [social-transit-solver-single](https://github.com/adam-rumpf/social-transit-solver-single). See the other repositories' READMEs for format details.

## Output Folder

This program writes output files to a local `output/` folder. The following files are produced:

* `arc_data.txt`
* `initial_flows.txt`
* `initial_solution_log.txt`
* `line_metrics.txt`
* `node_data.txt`
* `stop_metrics.txt`
* `transit_data.txt`

The contents of these files will be explained below for their relevant sections.

## Loading Factor Calculation

This is part of the main C++ program meant for helping to tune the assignment model parameters. We are currently using a frequency-based user assignment model from [Spiess and Florian (1989)](https://www.researchgate.net/publication/222385476_Optimal_Strategies_A_New_Assignment_Model_for_Transit_Networks) which makes use of a tuning parameter `alpha` to penalize overcrowding. This program outputs the loading factor of each individual arc, expressed as the initial flow:capacity ratio for each arc (with both flows and line capacities being calculated from the network's initial fleet assignment).

This program outputs no files, but prints the maximum and average loading factors to the screen. Seeing values significantly in excess of `1.0` indicates that `alpha` should be increased (although it should be noted that loading factors of up to `1.25` or so are generally assumed to be reasonable during rush hour).

## Stop-Level Metrics

This is part of the main C++ program meant for help in selecting candidate express routes. Our assumption is that a "good" express route would make it easier to get from a low-access area to a high-access area, and so as a preliminary step we calculate the accessibility metrics of the transit stops, themselves. This is done with essentially the same code as the main solver, except that we treat transit stops as population centers rather than considering the population centers separately.

Two files are output by this program. The first is `stop_metrics.txt`, which simply contains a list of all stop node IDs along with their accessibility metric. The second is `line_metrics.txt`, which contains effectively the same information as the first file, but partitioned by line and sorted by access metric value in order to make it easier to find the highest and lowest few metrics for any given line.

## Candidate Express Route Generation

This is a Mathematica notebook separate from the main C++ program for use in generating candidate express routes. We have chosen to use Mathematica simply because its built-in graph functions make evaluating connectivity much easier.

The general idea behind express route generation is, for each line, to pick a subset of the original stops consisting of only the highest few and lowest few accessibility metrics from the `line_metrics.txt` file. The line arcs of the express route are generated by finding the shortest paths between the corresponding stops in the original line, with a small amount of travel time deducted for each stop skipped.

The network definition files (`arc_data.txt`, `initial_flows.txt`, `node_data.txt`, and `transit_data.txt`) are all updated to include the express routes, each of which is assumed to begin with a fleet size of `0`. The updated files are placed in the `output/` folder in order to preserve the original versions.

The parameters for the express route generation process are defined and explained in the notebook file.
