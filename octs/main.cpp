#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <queue>
#include <numeric>
#include <time.h>
#include <tuple>
#define NOMINMAX
#include <windows.h>


using namespace std;

// ORTOOLS definition
#include ".\ortools\linear_solver\linear_solver.h"
// Configuration file contains static variables that can be tuned by the user
#include "config.h"
#include "structures.h"
// Objects utilized in photonics
#include "photo_device.h"
#include "waveguide.h"
#include "bend.h"
#include "photodetector.h"
#include "cross.h"
#include "splitter1x2.h"
#include "splitter1x3.h"
#include "splitter1x5.h"
#include "tunable_splitter1x2.h"
#include "splitter_input_mapping.h"
// Useful function collection
#include "functions.h"

//! Old deprecated approaches 
//#include "grouping.h" 
//#include "dp_grouping.h"
//#include "overlap_cross_handling.h"

// Function that finds the locations of the Merging Points with Linear Optimization
#include "optimization.h"
// Functions that focus on traversing the clock tree to find information on power and delay
#include "DFS.h"
// Header that contains functions initializing the groupings
#include "initGroupingMethods.h"
// Header that contains the grouping method utilized by the algorithm
#include "COP-KMeans.h"
// Functions that creates the routing in the clock tree
#include "routingDP.h"
#include "connections.h"
// Header that includes functions that create the topology vector
#include "TopologyVectorCreation.h"
// Header that handles cases that do not satisfy the user defined constraints of power can time
// To be implemented
//#include "checkInfeasibility.h"
//#include "spiral_creation.h"

// -----------------------------------------------------------------------------------------------------------------------

static tuple<vector<nary_tree_node*>, vector<int>> fullTreeConstruction(
    unordered_map<point, vector<int>, pointHash> point_dev_map,
    unordered_map<point, vector<double>, pointHash> dist_min_max,
    unordered_map<point, vector<double>, pointHash> power_min_max,
    vector<point> sinks,
    vector<nary_tree_node*> initial_noding,
    vector<vector<vector<int>>> topology) {

    // Create the photodetector nodes of the tree
    vector<nary_tree_node*> this_level = initial_noding;

    // Component numbering initialization
    vector<int> nums = { 0, 0, 0, 0, int(this_level.size()), 0, 0, 0 }; 

    //Initialization of the new groups after internal clustering
    unordered_map<int, vector<point>> new_group;
    point one_point = { -1, -1 };
    vector<point> multiple_points;

    // Remaining initialization
    unordered_map<point, vector<double>, pointHash> rem_dist_min_max;
    unordered_map<point, vector<double>, pointHash> rem_power_min_max;
    unordered_map<point, vector<int>, pointHash> rem_point_dev_map;
    unordered_map<int, vector<point>> rem_group;
    vector<nary_tree_node*> rem_node;

    // Preemptive Grouping
    if (debugger) {
        setTextColor(BROWN);
        cout << "-+-ITERATION 0-+-" << endl;
        setTextColor(YELLOW);
        cout << "Commences grouping...." << endl;
        setTextColor(WHITE);
    }
    time_t gstart = clock();
    unordered_map<int, vector<point>> group = topCopKMeansClustering(point_dev_map, topology[0]);
    time_t gend = clock();
    if (timer) {
        double cputime = (gend - gstart) / double(CLOCKS_PER_SEC);
        setTextColor(CYAN);
        cout << "Elapsed Time for Grouping is " << cputime << " seconds." << endl;
        setTextColor(WHITE);
    }
    if (debugger) {
        setTextColor(GREEN);
        cout << "Finished grouping!" << endl;
        setTextColor(WHITE);
    }

    double total_dist = 0;

    // While haven't reached single MP
    for (int i = 0; i < topology.size(); i++) {
        size_t group_size = group.size();
        int group_number = static_cast<int>(group_size);
        unordered_map<int, int> rotations;

        group = roundMappedVectoredPoints(group, precision);
        
        if (debugger) {
            cout << "---" << endl;
            cout << "Grouping results:" << endl;
            for (const auto& points : group) {
                cout << points.first << ":" << endl;
                for (const auto& pointy : points.second) {
                    cout << pointy << endl;
                }
            }
            cout << "---" << endl;
        }

        // Optimization Stage
        if (debugger) {
            setTextColor(YELLOW);
            cout << "Commences linear optimization...." << endl;
            setTextColor(WHITE);
        }
        time_t ostart = clock();
        tuple<vector<point>, unordered_map<point, double, pointHash>> collection = operations_research::optimizer_mrs(group, point_dev_map, power_min_max, dist_min_max, group_number);  
        time_t oend = clock();
        if (timer) {
            double cputime = (oend - ostart) / double(CLOCKS_PER_SEC);
            setTextColor(CYAN);
            cout << "Elapsed Time for Linear Optimization is " << cputime << " seconds." << endl;
            setTextColor(WHITE);
        }
        if (debugger) {
            setTextColor(GREEN);
            cout << "Finished linear optimization!" << endl;
            setTextColor(WHITE);
        }

        // Get the collection of new points from optimization
        vector<point> next_points = get<0>(collection);
        unordered_map<point, double, pointHash> SRs = get<1>(collection);
        next_points = roundAllPoints(next_points, precision);

        // Create a map of integer category to each new rounded point
        unordered_map<int, point> new_points;
        for (int i = 0; i < next_points.size(); i++) {
            new_points[i] = next_points[i];
        }

        unordered_map<point, vector<int>, pointHash> new_point_dev_map;
        int num = 0;
        int d = 0;

        // Updates the device map for the new points
        // Needed for the new early grouping
        for (const auto& groupy : group) {
            for (const auto& pointy : groupy.second) {
                for (const auto& devy : point_dev_map) {
                    if (devy.first == pointy) {
                        d += devy.second[3];
                    }
                }
            }
            new_point_dev_map[next_points[num]] = { -1, -1, -1, d};
            d = 0;
            num++;
        }

        if (debugger) {
            cout << "---" << endl;
            for (const auto& devs : new_point_dev_map) {
                cout << devs.first << endl;
                cout << "New number of sinks: " << devs.second[3] << endl;
            }
            cout << "---" << endl;
        }

        // New grouping is done early for splitter rotation finding purposes and grouping adjustments
        // Runs only if we haven't reached the last MP
        if (i != topology.size() - 1) {
            if (debugger) {
                setTextColor(YELLOW);
                cout << "Commences post-linear optimization grouping...." << endl;
                setTextColor(WHITE);
            }
            new_group = topCopKMeansClustering(new_point_dev_map, topology[i + 1]);
            time_t gend = clock();
            if (timer) {
                double cputime = (gend - gstart) / double(CLOCKS_PER_SEC);
                setTextColor(CYAN);
                cout << "Elapsed Time for post-linear optimization grouping is " << cputime << " seconds." << endl;
                setTextColor(WHITE);
            }
            if (debugger) {
                setTextColor(GREEN);
                cout << "Finished post-linear optimization grouping!" << endl;
                setTextColor(WHITE);
            }

            // Finds the rotations of the splitters
            rotations = findRotationExp(new_group, group, new_points);

            unordered_map<int, vector<point>> newest_groups;
            num = 0;
            for (const auto& grouppy : new_group) {
                vector<point> newest_group;
                for (const auto& pointy : grouppy.second) {
                    for (int k = 0; k < new_points.size(); k++) {
                        if (new_points[k] == pointy) {
                            if (rotations[k] == 0) {
                                newest_group.push_back({ pointy.x_coord, pointy.y_coord - 0.5 });
                            }
                            else if (rotations[k] == 90) {
                                newest_group.push_back({ pointy.x_coord - 0.5, pointy.y_coord });
                            }
                            else if (rotations[k] == 180) {
                                newest_group.push_back({ pointy.x_coord, pointy.y_coord + 0.5 });
                            }
                            else if (rotations[k] == 270) {
                                newest_group.push_back({ pointy.x_coord + 0.5, pointy.y_coord });
                            }
                            else if (rotations[k] == -1) {
                                newest_group.push_back(pointy);
                            }
                        }
                    }
                }
                newest_groups[num] = newest_group;
                num++;
            }
            new_group = newest_groups;
        }
        else {
            if (group[0].size() == 2) {
                rotations[0] = -1;
            }
            else {
                rotations[0] = 0;
            }
        }

        // Routing stage
        if (debugger) {
            setTextColor(YELLOW);
            cout << "Commences routing...." << endl;
            setTextColor(WHITE);
        }
        time_t rstart = clock();
        tuple<vector<nary_tree_node*>, vector<int>> inter_all =
            create_connections(new_points, group, this_level, nums, rotations, point_dev_map, SRs, new_group);
        time_t rend = clock();
        if (timer) {
            double cputime = (rend - rstart) / double(CLOCKS_PER_SEC);
            setTextColor(CYAN);
            cout << "Elapsed Time for routing is " << cputime << " seconds." << endl;
            setTextColor(WHITE);
        }
        if (debugger) {
            setTextColor(GREEN);
            cout << "Finished routing!" << endl;
            setTextColor(WHITE);
        }
        this_level = get<0>(inter_all);
        nums = get<1>(inter_all);

        num = 0;
        unordered_map<int, point> adjs_new_points;

        for (const auto& groupy : new_points) {
            if (rotations[num] == 0) {
                adjs_new_points[num] = { new_points[num].x_coord, new_points[num].y_coord - 0.5 };
            }
            else if (rotations[num] == 90) {
                adjs_new_points[num] = { new_points[num].x_coord - 0.5, new_points[num].y_coord };
            }
            else if (rotations[num] == 180) {
                adjs_new_points[num] = { new_points[num].x_coord, new_points[num].y_coord + 0.5 };
            }
            else if (rotations[num] == 270) {
                adjs_new_points[num] = { new_points[num].x_coord + 0.5, new_points[num].y_coord };
            }
            else {
                adjs_new_points[num] = new_points[num];
            }
            num++;
        }

        new_points = adjs_new_points;
        new_points = roundMappedPoints(new_points, precision);

        // Calculating the delays, power losses and number of devices
        if (debugger) {
            setTextColor(YELLOW);
            cout << "Commences map creation and SR adjustment...." << endl;
            setTextColor(WHITE);
        }
        time_t mstart = clock();
        // Adjust the SRs 
        unordered_map<point, vector<point>, pointHash> grps = groupCreator(group, new_points);
        splitterReplacer(power_min_max, this_level, grps, i);
        SR_adjustment(this_level, power_min_max);
        power_min_max = top_power_per_sink(this_level, sinks); // potentially slow step
        dist_min_max = top_delay_per_sink(dist_min_max, group, new_points, this_level);
        point_dev_map = top_count(point_dev_map, group, new_points, this_level);
        time_t mend = clock();
        if (timer) {
            double cputime = (mend - mstart) / double(CLOCKS_PER_SEC);
            setTextColor(CYAN);
            cout << "Elapsed Time for new map creation and SR adjustment is " << cputime << " seconds." << endl;
            setTextColor(WHITE);
        }
        if (debugger) {
            setTextColor(GREEN);
            cout << "Finished map creation and SR adjustment!" << endl;
            setTextColor(WHITE);
        }

        // Necessary to save the sole point for the next iteration
        if (multiple_points.size() != 0) {
            int num = 0;
            for (const auto& pointy : multiple_points) {
                dist_min_max[pointy] = rem_dist_min_max[pointy];
                power_min_max[pointy] = rem_power_min_max[pointy];
                point_dev_map[pointy] = rem_point_dev_map[pointy];
                this_level.push_back(rem_node[num]);
                num++;
            }
            one_point = { -1, -1 };
            rem_dist_min_max.clear();
            rem_power_min_max.clear();
            rem_point_dev_map.clear();
            rem_node.clear();
            multiple_points.clear();
        }

        for (const auto& points : new_group) {
            if (points.second.size() == 1) {
                one_point = points.second[0];
                multiple_points.push_back(one_point);
                rem_dist_min_max[one_point] = dist_min_max[one_point];
                rem_power_min_max[one_point] = power_min_max[one_point];
                rem_point_dev_map[one_point] = point_dev_map[one_point];
                for (const auto& nodes : this_level) {
                    if (nodes->dev->print_loc1() == one_point || nodes->dev->print_loc2() == one_point) {
                        rem_node.push_back(nodes);
                    }
                }
            }
        }

        group = new_group;
        
        if (debugger) {
            setTextColor(BROWN);
            cout_maps(point_dev_map, dist_min_max, power_min_max);
            if (i != topology.size() - 1) {
                cout << "-+-ITERATION " << i + 1 << "-+-" << endl;
                setTextColor(WHITE);
            }
        }
    }

    return make_tuple(this_level, nums);
}

// -----------------------------------------------------------------------------------------------------------------------


int main() {

    // Load the sink locations
    vector<point> sinks = extract_coordinates(input_filename);
    size_t sink_number = sinks.size();
    int sn = static_cast<int>(sink_number);
    vector<int> primes = prime_factorization(sn);
    sort(primes.begin(), primes.end(), greater<int>());
    vector<vector<vector<int>>> top_vect = topology_vector_creation(primes, sn, 20);

    if (debugger) {
        cout << "Number of sinks: " << sn << endl;
        cout << "Primes: " << primes << endl;
        cout << "Groupings: " << endl;
        cout << top_vect << endl;
    }

    //! Deprecated code 
    /*
    // Load the topology vector
    string filename2 = "./Libraries/library"+to_string(sinks.size())+".txt";
    //vector<vector<vector<int>>> top_vect = get_topology_vector(filename2);
    */

    // Initialize the maps and the vector of nodes with photodetectors
    unordered_map<point, vector<int>, pointHash> point_dev_map = initialize_map(sinks);
    unordered_map<point, vector<double>, pointHash> dist_min_max = initialize_map2(sinks);
    unordered_map<point, vector<double>, pointHash> power_min_max = initialize_map2(sinks);
    vector<nary_tree_node*> initial_nodes = photo_init(sinks);

    // This tuple contains the solution of OCTS
    tuple<vector<nary_tree_node*>, vector<int>> final_result;

    // Run OCTS
    setTextColor(YELLOW);
    cout << "-----------------------------" << endl;
    cout << "---Starts CTS Construction---" << endl;
    cout << "-----------------------------" << endl;
    setTextColor(WHITE);
    time_t tstart = clock();
    final_result = fullTreeConstruction(point_dev_map, dist_min_max, power_min_max, sinks, initial_nodes, top_vect);
    time_t tend = clock();
    setTextColor(YELLOW);
    cout << "----------------------------" << endl;
    cout << "---Ended CTS Construction---" << endl;
    cout << "----------------------------" << endl;
    setTextColor(WHITE);
    vector<nary_tree_node*> final_nodes = get<0>(final_result);
    vector<int> comp_nums = get<1>(final_result);

    // Print the results
    // Prints a txt file containing the locations of the components and a verilog file
    ofstream output_file(output_filename);
    ofstream output_file_verilog(output_filename_verilog);
    fullTreePrinter(final_nodes[0], output_file, comp_nums);
    verilogCreator(final_nodes[0], output_file_verilog, comp_nums);
    if (timer) {
        double cputime = (tend - tstart) / double(CLOCKS_PER_SEC);
        setTextColor(BLUE);
        cout << "Elapsed Time is " << cputime << " seconds." << endl;
        setTextColor(GREEN);
        cout << "Finished!" << endl;
    }
    return 0;
}

// -----------------------------------------------------------------------------------------------------------------------
//! Deprecated code
/*
tuple<vector<nary_tree_node*>, vector<int>> full_tree_construction(
    unordered_map<point, vector<int>, pointHash> point_dev_map,
    unordered_map<point, vector<double>, pointHash> dist_min_max,
    unordered_map<point, vector<double>, pointHash> power_min_max,
    vector<point> sinks,
    vector<nary_tree_node*> initial_noding,
    vector<vector<vector<int>>> topology) {

    vector<nary_tree_node*> this_level = initial_noding;
    vector<int> nums = { 0, 0, 0, 0, int(this_level.size()) }; // Component numbers
    unordered_map<int, vector<point>> new_group;

    // Part1
    vector<vector<int>> small_top = topology[0];
    cout << "Before grouping" << endl;
    unordered_map<int, vector<point>> group = top_grouping(point_dev_map, small_top);
    cout << "Final Grouping:\n";

    for (const auto& pair : group) {
        int key = pair.first;
        const std::vector<point>& points = pair.second;

        std::cout << "Group: " << key << std::endl;
        std::cout << "Points: [ ";
        for (const point& point : points) {
            std::cout << "(" << point.x_coord << ", " << point.y_coord << ") ";
        }
        std::cout << "]" << std::endl;
    }

    for (int i = 0; i < topology.size(); i++) {

        bool same_sizes = areAllVectorsSameSize(topology[i]);

        // Part2
        int number = find_number_of_points(group);
        cout << "This number = " << number << endl;
        unordered_map<int, int> rotations;
        for (const auto& pair : rotations) {
            cout << pair.first << " : " << pair.second << endl;
        }
        vector<point> next_points = operations_research::optimizer_mrs(group, point_dev_map, power_min_max, dist_min_max, number);
        next_points = roundAllPoints(next_points, 10);
        unordered_map<int, point> new_points;

        // Adjust the points based on the group size
        for (int i = 0; i < next_points.size(); i++) {
            new_points[i] = next_points[i];
        }

        unordered_map<point, vector<int>, pointHash> new_point_dev_map;
        int num = 0;
        for (const auto& groupy : group) {
            for (const auto& pointy : groupy.second) {
                for (const auto& devy : point_dev_map) {
                    if (devy.first == pointy) {
                        new_point_dev_map[next_points[num]] = { devy.second[0], devy.second[1], devy.second[2], int(devy.second[3] * groupy.second.size()) };
                        cout << "CHECK SINK NUMBER = " << int(devy.second[3] * groupy.second.size()) << endl;
                    }
                }
            }
            num++;
        }

        cout << "HRE" << endl;

        if (i != topology.size() - 1) {
            new_group = top_grouping(new_point_dev_map, topology[i + 1]);
            cout << "grouping done" << endl;
            for (const auto& pair :new_group) {
                int key = pair.first;
                const std::vector<point>& points = pair.second;

                std::cout << "Group: " << key << std::endl;
                std::cout << "Points: [ ";
                for (const point& point : points) {
                    std::cout << "(" << point.x_coord << ", " << point.y_coord << ") ";
                }
                std::cout << "]" << std::endl;
            }
            cout << "--- " << endl;
            for (const auto& pair : group) {
                int key = pair.first;
                const std::vector<point>& points = pair.second;

                std::cout << "Group: " << key << std::endl;
                std::cout << "Points: [ ";
                for (const point& point : points) {
                    std::cout << "(" << point.x_coord << ", " << point.y_coord << ") ";
                }
                std::cout << "]" << std::endl;
            }
            if (i == 1) { abort(); }
            rotations = find_rotation_exp(new_group, group, new_points);
            cout << "rotations done" << endl;
            unordered_map<int, vector<point>> newest_groups;
            for (const auto& rot : rotations) {
                cout << rot.first << " : " << rot.second << endl;
            }
            num = 0;
            for (const auto& grouppy : new_group) {
                vector<point> newest_group;
                for (const auto& pointy : grouppy.second) {
                    for (int k = 0; k < new_points.size(); k++) {
                        if (new_points[k] == pointy) {
                            if (rotations[k] == 0) {
                                newest_group.push_back({ pointy.x_coord, pointy.y_coord - 0.5 });
                            }
                            else if (rotations[k] == 90) {
                                newest_group.push_back({ pointy.x_coord - 0.5, pointy.y_coord });
                            }
                            else if (rotations[k] == 180) {
                                newest_group.push_back({ pointy.x_coord, pointy.y_coord + 0.5 });
                            }
                            else if (rotations[k] == 270) {
                                newest_group.push_back({ pointy.x_coord + 0.5, pointy.y_coord });
                            }
                            else if (rotations[k] == -1) {
                                newest_group.push_back(pointy);
                            }
                        }
                    }
                }
                newest_groups[num] = newest_group;
                num++;
            }
            new_group = newest_groups;
        }
        else {
            if (group[0].size() == 2) {
                rotations[0] = -1;
            }
            else {
                rotations[0] = 0;
            }
        }

        for (const auto& groupy : new_group) {
            for (const auto& pointy : groupy.second) {
                cout << groupy.first << " : " << pointy << endl;
            }
        }

        cout << "Part 3" << endl;
        // Part3
        // Save the groups that have 1 element in them for the next iteration

        tuple inter_all =
            create_connections(new_points, group, this_level, find_number_of_points(group), nums, rotations);
        this_level = get<0>(inter_all);
        nums = get<1>(inter_all);

        num = 0;
        unordered_map<int, point> adjs_new_points;

        for (const auto& groupy : new_points) {
            if (rotations[num] == 0) {
                adjs_new_points[num] = { new_points[num].x_coord, new_points[num].y_coord - 0.5 };
            }
            else if (rotations[num] == 90) {
                adjs_new_points[num] = { new_points[num].x_coord - 0.5, new_points[num].y_coord };
            }
            else if (rotations[num] == 180) {
                adjs_new_points[num] = { new_points[num].x_coord, new_points[num].y_coord + 0.5 };
            }
            else if (rotations[num] == 270) {
                adjs_new_points[num] = { new_points[num].x_coord + 0.5, new_points[num].y_coord };
            }
            else {
                adjs_new_points[num] = new_points[num];
            }
            num++;
        }

        new_points = adjs_new_points;

        // Part4
        power_min_max = top_power_per_sink(this_level, sinks, rotations);
        dist_min_max = top_delay_per_sink(dist_min_max, group, new_points, this_level, rotations);
        point_dev_map = top_count(point_dev_map, group, new_points, this_level, rotations);
        group = new_group;

        for (const auto& pair : power_min_max) {
            cout << "Key: " << pair.first << ", Container: ";

            for (const auto& value : pair.second) {
                cout << value << " ";
            }

            cout << endl;
        }

        cout << "---" << endl;

        for (const auto& pair : dist_min_max) {
            cout << "Key: " << pair.first << ", Container: ";

            for (const auto& value : pair.second) {
                cout << value << " ";
            }

            cout << endl;
        }

        cout << "---" << endl;

        for (const auto& pair : point_dev_map) {
            cout << "Key: " << pair.first << ", Container: ";

            for (const auto& value : pair.second) {
                cout << value << " ";
            }

            cout << endl;
        }

        cout << "---" << endl;
    }

    return make_tuple(this_level, nums);
}

for (const auto& points : group2) {
    double midx = 0;
    double midy = 0;
    for (const auto& pointy : points.second) {
        midx += pointy.x_coord;
        midy += pointy.y_coord;
    }
    midx /= points.second.size();
    midy /= points.second.size();
    point mid = { midx, midy };
    if (points.second.size() == 2) {
        total_dist += (manhattan_distance(points.second[0], mid) + manhattan_distance(points.second[1], mid));
    }
    else if (points.second.size() == 3) {
        total_dist += (manhattan_distance(points.second[0], mid) + manhattan_distance(points.second[1], mid) + manhattan_distance(points.second[2], mid));
    }
    else if (points.second.size() == 5) {
        total_dist += (manhattan_distance(points.second[0], mid) + manhattan_distance(points.second[1], mid) + manhattan_distance(points.second[2], mid)
            + manhattan_distance(points.second[3], mid) + manhattan_distance(points.second[4], mid));
    }
}

cout << total_dist << endl;
abort();
*/

