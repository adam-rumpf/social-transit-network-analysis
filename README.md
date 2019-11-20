# social-transit-network-analysis

Preprocessing tools for analyzing the input network for a research project of mine dealing with a public transit design model with social access objectives. This is meant for use in analyzing the network after preprocessing with [social-transit-solver-single](https://github.com/adam-rumpf/social-transit-solver-single), with which this repository shares a large amount of code.

Specifically, this is a set of programs meant for performing the following tasks:

* (C++) Calculate the initial flow:capacity ratio of every arc in the network. This is to determine whether the assignment model parameters are reasonable, since a ratio much greater than 1.0 would suggest that overcrowding should be more harshly penalized.
* (C++) Calculate the accessibility metric for every stop. This is done by reusing the main solution algorithm's accessibility metric code, but treating the transit stops, themselves, as the population centers.
* (Mathematica) Generate candidate express routes using the stop accessibility metrics. This is done for each route by taking a subset of the stops with the highest and lowest accessibility metrics and creating a route that uses only those stops.

We use a separate Mathematica file for candidate route generation simply because its graph objects make it much simpler to evaluate network connectivity.
