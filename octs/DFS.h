// Functions used for routing and for calculating the new power/delay maps
// Most functions use DFS (Depth First Search)
// -----------------------------------------------------------------------------------------------------------------------

// First DFS for cross detection
vector<point> DFS_cross(nary_tree_node* parent, nary_tree_node* node, vector<point> &crosses) {
    /**
    * DFS function for cross detection, checking to find the waveguide with which there is a cross
    * Necessary to create the cross object on the crossed waveguide as well
    * @param parent Parent of the node "node"
    * @param node The node being searched
    * @param crosses A vector of crosses that need to be found
    */

    // Enters if the node corresponds to a waveguide
    if (node->dev->object_id == 1) {
        point xy_start = node->dev->print_loc1();
        point xy_end = node->dev->print_loc2();
        // If a cross is found
        point cr = { -1, -1 };
        for (const auto& crs : crosses) {
            //cout << crs << endl;
            bool one = (crs.x_coord == xy_start.x_coord && crs.x_coord == xy_end.x_coord &&
                crs.y_coord > min(xy_start.y_coord, xy_end.y_coord) && crs.y_coord < max(xy_start.y_coord, xy_end.y_coord));
            bool two = (crs.y_coord == xy_start.y_coord && crs.y_coord == xy_end.y_coord &&
                crs.x_coord > min(xy_start.x_coord, xy_end.x_coord) && crs.x_coord < max(xy_start.x_coord, xy_end.x_coord));
            if (one || two) {
                cr = crs;
            }
        }
        if (cr != point{ -1, -1 }) {
            // Removes a child to add a cross
            remove_child_cross(parent, node, cr);
        }
    }

    // Returns if no children present
    if (node->children.size() == 0 || node == NULL) {
        return crosses;
    }

    // Repeat for searching each child
    for (int i = 0; i < node->children.size(); i++) {
        crosses = DFS_cross(node, node->children[i], crosses);
    }

    return crosses;
}

// -----------------------------------------------------------------------------------------------------------------------

// first vector -> horizontal, second vector -> vertical
// filters out the points (starter == ender)
pair<vector<line>, vector<line>> DFSWindowSearcher(nary_tree_node* node, pair<vector<line>, vector<line>> window, point p1, point p2) {
    /**
    * Recursive function that saves all the lines in a window (between two points)
    * The window is defined by considering the first point as bottom left and the second point as top right
    * The window can be a single line if the two points have the same X or Y
    * @param node The examined node
    * @param window A pair of horizontal and vertical lines that is filled recursively
    * @param p1 The first point (bottom left)
    * @param p2 The second point (top right)
    */

    // Enters if the node corresponds to a waveguide
    if (node->dev->object_id == 1) {
        point xy_start = node->dev->print_loc1();
        point xy_end = node->dev->print_loc2();

        // Checks if the waveguide (or part of it) is inside the window
        double lo_x = min(p1.x_coord, p2.x_coord);
        double lo_y = min(p1.y_coord, p2.y_coord);
        double up_x = max(p1.x_coord, p2.x_coord);
        double up_y = max(p1.y_coord, p2.y_coord);
        bool check_horiz = ((xy_start.y_coord >= lo_y) && (xy_start.y_coord <= up_y) && (((xy_start.x_coord <= up_x) && (xy_end.x_coord >= lo_x))
            || ((xy_end.x_coord <= up_x) && (xy_start.x_coord >= lo_x))));
        bool check_vert = ((xy_start.x_coord >= lo_x) && (xy_start.x_coord <= up_x) && (((xy_start.y_coord <= up_y) && (xy_end.y_coord >= lo_y))
            || ((xy_end.y_coord <= up_y) && (xy_start.y_coord >= lo_y))));
        bool horiz = line{ xy_start, xy_end }.is_horizontal();
        bool vert = line{ xy_start, xy_end }.is_vertical();

        // All possible line positions along with locations of xy_start and xy_end
        // Horizontal check
        if (horiz && check_horiz) {
            point starter, ender;
            if (xy_start.x_coord <= lo_x && (xy_end.x_coord >= lo_x && xy_end.x_coord <= up_x)) {
                starter = { lo_x, xy_start.y_coord };
                ender = {xy_end.x_coord, xy_end.y_coord};
            }
            else if ((xy_start.x_coord <= up_x && xy_start.x_coord >= lo_x) && xy_end.x_coord >= up_x) {
                starter = { xy_start.x_coord, xy_start.y_coord };
                ender = { up_x, xy_end.y_coord };
            }
            else if (xy_end.x_coord <= lo_x && (xy_start.x_coord >= lo_x && xy_start.x_coord <= up_x)) {
                starter = { lo_x, xy_end.y_coord };
                ender = { xy_start.x_coord, xy_start.y_coord };
            }
            else if ((xy_end.x_coord <= up_x && xy_end.x_coord >= lo_x) && xy_start.x_coord >= up_x) {
                starter = { xy_end.x_coord, xy_end.y_coord };
                ender = { up_x, xy_start.y_coord };
            }
            else if ((xy_start.x_coord <= lo_x && xy_end.x_coord >= up_x) ||
                (xy_end.x_coord <= lo_x && xy_start.x_coord >= up_x)) {
                starter = { lo_x, xy_start.y_coord };
                ender = { up_x, xy_end.y_coord };
            }
            else {
                if (xy_start.x_coord > xy_end.x_coord) {
                    starter = xy_end;
                    ender = xy_start;
                }
                else {
                    starter = xy_start;
                    ender = xy_end;
                }
            }
            // Test first if the start/end points are the same and they are not at the p1 or p2 points
            // This case is necessary if the window is 1D
            if ((starter == ender && starter != p1 && starter != p2)) {
                // Check if the points p1 and p2 have the same x or y coordinate
                // If neither, then ignore the line
                if (p1.x_coord == p2.x_coord) {
                    window.first.push_back(line{ point{starter.x_coord + 1e-4, starter.y_coord}, point{starter.x_coord - 1e-4, starter.y_coord} });
                }
                else if (p1.y_coord == p2.y_coord) {
                    window.first.push_back(line{ point{starter.x_coord, starter.y_coord + 1e-4}, point{starter.x_coord, starter.y_coord - 1e-4} });
                }
            }
            // In case the start and end are not the same
            if (starter != ender) {
                window.first.push_back(line{ starter, ender });
            }
            // Vertical check
        } else if (vert && check_vert) {
            point starter, ender;
            if (xy_start.y_coord <= lo_y && (xy_end.y_coord >= lo_y && xy_end.y_coord <= up_y)) {
                starter = { xy_start.x_coord, lo_y };
                ender = { xy_end.x_coord, xy_end.y_coord };
            }
            else if ((xy_start.y_coord <= up_y && xy_start.y_coord >= lo_y) && xy_end.y_coord >= up_y) {
                starter = { xy_start.x_coord, xy_start.y_coord };
                ender = { xy_end.x_coord, up_y };
            }
            else if (xy_end.y_coord <= lo_y && (xy_start.y_coord >= lo_y && xy_start.y_coord <= up_y)) {
                starter = { xy_end.x_coord, lo_y };
                ender = xy_start;
            }
            else if ((xy_end.y_coord <= up_y && xy_end.y_coord >= lo_y) && xy_start.y_coord >= up_y) {
                starter = xy_end;
                ender = { xy_start.x_coord, up_y };
            }
            else if ((xy_start.y_coord <= lo_y && xy_end.y_coord >= up_y) ||
                (xy_end.y_coord <= lo_y && xy_start.y_coord >= up_y)) {
                starter = { xy_start.x_coord, lo_y };
                ender = { xy_end.x_coord, up_y };
            }
            else {
                if (xy_start.y_coord > xy_end.y_coord) {
                    starter = xy_end;
                    ender = xy_start;
                }
                else {
                    starter = xy_start;
                    ender = xy_end;
                }
            }
            // Test first if the start/end points are the same and they are not at the p1 or p2 points
            // This case is necessary if the window is 1D
            if ((starter == ender && starter != p1 && starter != p2)) {
                // Check if the points p1 and p2 have the same x or y coordinate
                // If neither, then ignore the line
                if (p1.x_coord == p2.x_coord) {
                    window.second.push_back(line{ point{starter.x_coord + 1e-4, starter.y_coord}, point{starter.x_coord - 1e-4, starter.y_coord} });
                }
                else if (p1.y_coord == p2.y_coord) {
                    window.second.push_back(line{ point{starter.x_coord, starter.y_coord + 1e-4}, point{starter.x_coord, starter.y_coord - 1e-4} });
                }
            }
            if (starter != ender) {
                window.second.push_back(line{ starter, ender });
            }
        }
    }

    // Returns if no children present
    if (node->children.size() == 0 || node == NULL) {
        return window;
    }

    // Recursive call for each child
    for (int i = 0; i < node->children.size(); i++) {
        window = DFSWindowSearcher(node->children[i], window, p1, p2);
    }

    return window;
}

// -----------------------------------------------------------------------------------------------------------------------

void top_DFS_cross(vector<nary_tree_node*> &tops, vector<point> all_crosses) {
    /**
    * Cross top function, gets the node tree and the waveguide to be tested
    * @param tops The top nodes of the subtress of the currently examined level
    * @param all_crosses The vector of points corresponding to locations of crosses
    */

    // For each subtree check for crosses
    for (int i = 0; i < tops.size(); i++) {
        all_crosses = DFS_cross(tops[i], tops[i], all_crosses);
    }

}

// -----------------------------------------------------------------------------------------------------------------------
// DFS Runs for calculating the delay, power loss and number of components
// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

tuple<vector<double>, vector<point>> delay_per_sink(nary_tree_node* node,
    vector<point> prev_level, tuple<vector<double>, vector<point>> this_one, int index) {
    /**
    * DFS function for calculating the delay thus far
    * @param node The current node
    * @param prev_level The point of the previous lever of the tree
    * @param this_one Tuple of the delays and the points
    * @param index Integer index showing which delay to take into account
    */

    vector<point>& sequence = get<1>(this_one);
    vector<double>& delay = get<0>(this_one);
    bool found = false;

    for (const auto& item : prev_level) {
        //cout << object_rec(node->dev) << endl;
        //cout << " Loc1: " << node->dev->print_loc1() << endl;
        //cout << " Loc2: " << node->dev->print_loc2() << endl;
        if (item == node->dev->print_loc1()) {
            found = true;
            sequence.push_back(node->dev->print_loc1());
            //cout << "-----------------------------------------------" << endl;
            //cout << node->dev->print_loc1() << endl;
            //cout << "delay = " << delay[sequence.size() - 1] << endl;
            break;
        }
    }

    if (found) {
        return this_one;
    }

    if (node->dev->object_id == 1 && node->children[0]->dev->object_id == 5) {
        vector<double>& delays = get<0>(this_one);
        for (int k = 0; k < delays.size(); k++) {
            delays[k] += node->dev->calc_delay();
        }
    } else if (node->dev->object_id == 1 && node->children[0]->dev->object_id != 5) {
        vector<double>& delays = get<0>(this_one);
        //cout << "This delay is " << node->dev->calc_delay() << endl;
        delays[index] += node->dev->calc_delay();
        //cout << "Total delay so far = " << delays[index] << endl;
    }

    // For each subtree check the delay
    for (int i = 0; i < node->children.size(); i++) {
        this_one = delay_per_sink(node->children[i], prev_level, this_one, index);
        index++;
    }

    return this_one;
}

// -----------------------------------------------------------------------------------------------------------------------
unordered_map<point, vector<double>, pointHash> top_delay_per_sink(
    unordered_map<point, vector<double>, pointHash> delays_min_max,
    unordered_map<int, vector<point>> prev_level,
    unordered_map<int, point> centers, vector<nary_tree_node*> tops) {
    /**
    * This is the top DFS function that calculates the delay towards each sink
    * @param delays_min_max The previous map of the min/max delays mapping them to each top point of the subtrees
    * @param prev_level The groups in the previous levels
    * @param centers The new MPs found
    * @param tops The top N-ary nodes so far from the subtrees
    */
    unordered_map<point, vector<double>, pointHash> new_delays_min_max;

    for (int i = 0; i < tops.size(); i++) {
        int found_key = -1;
        for (const auto& pair : centers) {
            //cout << "CENTERS" << pair.second << endl;
            //cout << tops[i]->dev->print_loc1() << endl;
            //cout << rotations[i] << endl;
            if (pair.second == tops[i]->dev->print_loc1()) {
                found_key = pair.first;
                //cout << "breaks" << endl;
                break;
            }
        }

        vector<double> delays;
        vector<point> this_vector = prev_level[found_key];

        if (tops[i]->dev->object_id == 2 || tops[i]->dev->object_id == 7) {
            /// Find if the key exists then extract info
            delays = { 0 , 0 };
        }
        else if (tops[i]->children[0]->dev->object_id == 5) {
            /// Find if the key exists then extract info
            delays = { 0 , 0 , 0 };
        }
        else if (tops[i]->children[0]->dev->object_id == 6) {
            /// Find if the key exists then extract info
            delays = { 0 , 0 , 0 , 0 , 0 };
        }

        tuple tu = delay_per_sink(tops[i], this_vector, { delays, {} }, 0);
        //cout << "Delay per sink ends here" << endl;
        delays = get<0>(tu);
        vector<point> sequence = get<1>(tu);
        double first_min = 0;
        double first_max = 0;

        vector<double> mins, maxs;
        //cout << "THIS VECTOR" << this_vector.size() << endl;
        for (int j = 0; j < this_vector.size(); j++) {
            auto it = delays_min_max.find(this_vector[j]);
            int index = -1;
            if (it != delays_min_max.end()) {
                auto it = find(sequence.begin(), sequence.end(), this_vector[j]);
                if (it != sequence.end()) {
                    index = static_cast<int>(distance(sequence.begin(), it));
                }
                first_min = delays[index] + delays_min_max[this_vector[j]][0];
                //cout << delays[index] << "+" << delays_min_max[this_vector[j]][0] << endl;
                //cout << "FIRST " << first_min << endl;
                first_max = delays[index] + delays_min_max[this_vector[j]][1];
                //cout << delays[index] << "+" << delays_min_max[this_vector[j]][1] << endl;
                //cout << "FIRST " << first_max << endl;
                mins.push_back(first_min);
                maxs.push_back(first_max);
            }
        }
        double this_min = *min_element(mins.begin(), mins.end());
        double this_max = *max_element(maxs.begin(), maxs.end());
        new_delays_min_max[tops[i]->dev->print_loc1()] = { this_min , this_max };
    }

    return new_delays_min_max;
}

// -----------------------------------------------------------------------------------------------------------------------

pair<unordered_map<point, double, pointHash>, tunable_splitter1x2*> power_per_sink(nary_tree_node* node,
    unordered_map<point, double, pointHash> power_point, point prev_loc, tunable_splitter1x2* tns1x2) {
    /**
    * Function that calculates the power consumption of the subtrees by using DFS
    * Returns a map of the top point of each subtree with the losses until their respective sinks 
    * Also returns a tunable splitter in case it is found from a previous node to calculate the power considering the SR
    * @param node The node of the subtree
    * @param power_point Map having the point and the power so far
    * @param prev_loc The previous point location
    * @param tns1x2 The tunable splitter pointer in case it is found
    */
    if (node == NULL) {
        return make_pair(power_point, tns1x2);
    }

    point loc;

    // In case the device is a waveguide
    if (node->dev->object_id == 1) {
        loc = node->dev->print_loc2();
    }
    else {
        loc = node->dev->print_loc1();
    }

    if (debugger) {
        cout << "Device = " << object_rec(node->dev) << endl;
        cout << "Device loc = " << node->dev->print_loc1() << " " << node->dev->print_loc2() << endl;
        cout << "Power of device = " << node->dev->calc_loss() << endl;
    }

    power_point[loc] = power_point[prev_loc] - node->dev->calc_loss();

    // Adjusting the power based on the splitting ratio
    if (tns1x2) {
        if (tns1x2->get_path_sr1() == loc) {
            if (debugger) {
                cout << "Found a tunable splitter... considering the splitter, path 1.\n";
            }
            power_point[loc] = power_point[loc] + 10 * log10(tns1x2->get_sr());
            tns1x2->set_path_sr1({-1, -1});
        }
        else if (tns1x2->get_path_sr2() == loc) {
            if (debugger) {
                cout << "Found a tunable splitter... considering the splitter, path 2.\n";
            }
            power_point[loc] = power_point[loc] + 10 * log10(1 - tns1x2->get_sr());
            tns1x2->set_path_sr2({ -1, -1 });
        }
        if (tns1x2->get_path_sr1() == point{ -1 ,-1 } && tns1x2->get_path_sr2() == point{ -1, -1 }) {
            tns1x2 = nullptr;
        }
    }

    // Recognise the tunable splitter and save a pointer to it
    if (node->dev->object_id == 7) {
        tns1x2 = dynamic_cast<tunable_splitter1x2*>(node->dev);
    }

    if (debugger) {
        cout << "Power so far = " << power_point[loc] << endl;
        cout << "How many children = " << node->children.size() << endl;
    }

    for (int i = 0; i < node->children.size(); i++) {
        pair next_pair = power_per_sink(node->children[i], power_point, loc, tns1x2);
        power_point = get<0>(next_pair);
        tns1x2 = get<1>(next_pair);
    }

    return make_pair(power_point, tns1x2);
}

// -----------------------------------------------------------------------------------------------------------------------
//! Potentially this and previous function need to be optimized
unordered_map<point, vector<double>, pointHash> top_power_per_sink(vector<nary_tree_node*> level_nodes, vector<point> initial_sinks) {
    /**
    * Top function responsible for calculating the power consumption across the subtrees till each sink
    * The power consumption is computed bu adding the dB losses of each photonic component already added in the tree
    * @param level_nodes The current nodes of the subtrees
    * @param initial_sinks The points where the sinks are located
    */

    unordered_map<point, vector<double>, pointHash> power_min_max;
    for (int i = 0; i < level_nodes.size(); i++) {
        unordered_map<point, double, pointHash> this_power;
        point this_node;
        this_node = level_nodes[i]->dev->print_loc1();
        this_power[this_node] = 0;
        this_power = get<0>(power_per_sink(level_nodes[i], this_power, this_node, nullptr));
        vector<double> all_powers;
        for (int j = 0; j < initial_sinks.size(); j++) {
            auto it = this_power.find(initial_sinks[j]);
            if (it != this_power.end()) {
                all_powers.push_back(this_power[initial_sinks[j]]);
            }
        }
        double max_power = *max_element(all_powers.begin(), all_powers.end());
        double min_power = *min_element(all_powers.begin(), all_powers.end());
        power_min_max[this_node] = { min_power, max_power };
    }

    return power_min_max;
}

// -----------------------------------------------------------------------------------------------------------------------

vector<int> DFS_count(nary_tree_node* node, vector<point> group, vector<int> devs) {
    /**
    * Depth first search run that counts the components until it reaches a specific node
    * Basically the idea is the following:
    * We know the number of devices for each subtree for the current iteration
    * We add the devices from the grouped subtrees together
    * Then DFS is used to count the remaining number of devices from the currently found MP to the top of the subtrees that have been grouped together
    * This is done because we do not need to count all the devices each time, or else we waste iterations
    * 
    * @param node The node to be reached
    * @param group Group of points corresponding to the specific starting MP
    * @param devs Number of devices from the previously constructed subtrees
    */
    bool found = false;
    for (const auto& item : group) {
        if (item == node->dev->print_loc1()) {
            found = true;
            break;
        }
    }

    if (found) {
        return devs;
    }

    // Increase the respective counter depending on the device found
    if (node->dev->object_id == 0) {
        devs[0] += 1;
    }
    else if (node->dev->object_id == 3) {
        devs[1] += 1;
    }
    else if (node->dev->object_id == 2) {
        devs[2] += 1;
    }
    else if (node->dev->object_id == 5) {
        devs[4] += 1;
    }
    else if (node->dev->object_id == 6) {
        devs[5] += 1;
    }
    else if (node->dev->object_id == 7) {
        devs[6] += 1;
    }

    for (int i = 0; i < node->children.size(); i++) {
        devs = DFS_count(node->children[i], group, devs);
    }

    return devs;
}

// -----------------------------------------------------------------------------------------------------------------------

unordered_map<point, vector<int>, pointHash> top_count(
    unordered_map<point, vector<int>, pointHash> prev_count,
    unordered_map<int, vector<point>> prev_level,
    unordered_map<int, point> centers, vector<nary_tree_node*> tops) {
    /**
    * Top function for counting the number of devices in the clock tree
    * Each counter signifies the number of devices, saved in a respective position based on the device ID
    * 
    * @param prev_count Previous map with the count of devices for each point
    * @param prev_level Previous groupings of points
    * @param centers MPs corresponding to each group
    * @param tops Currently created subtrees
    */

    unordered_map<point, vector<int>, pointHash> new_count;

    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int count4 = 0;
    int count5 = 0;
    int count6 = 0;
    int count7 = 0;

    for (int i = 0; i < tops.size(); i++) {
        // Get the group
        int found_key = -1;
        for (const auto& pair : centers) {
            if (pair.second == tops[i]->dev->print_loc1()) {
                found_key = pair.first;
                break;
            }
        }
        vector<point> this_vector = prev_level[found_key];

        // Find if the key exists then extract info
        vector<int> count = { 0 , 0 , 0 , 0 , 0 , 0 , 0  };

        count = DFS_count(tops[i], this_vector, count);

        for (int j = 0; j < this_vector.size(); j++) {
            auto it = prev_count.find(this_vector[j]);
            if (it != prev_count.end()) {
                count1 += prev_count[this_vector[j]][0];
                count2 += prev_count[this_vector[j]][1];
                count3 += prev_count[this_vector[j]][2];
                count4 += prev_count[this_vector[j]][3];
                count5 += prev_count[this_vector[j]][4];
                count6 += prev_count[this_vector[j]][5];
                count7 += prev_count[this_vector[j]][6];
            }
        }
        count1 += count[0];
        count2 += count[1];
        count3 += count[2];
        count4 += count[3];
        count5 += count[4];
        count6 += count[5];
        count7 += count[6];
        new_count[tops[i]->dev->print_loc1()] = { count1, count2, count3, count4, count5, count6, count7 };
        count1 = 0;
        count2 = 0;
        count3 = 0;
        count4 = 0;
        count5 = 0;
        count6 = 0;
        count7 = 0;
    }

    return new_count;
}

// -----------------------------------------------------------------------------------------------------------------------

void SR_adjustment(vector<nary_tree_node*> level_nodes, unordered_map<point, vector<double>, pointHash> power_min_max) {
    /**
    * Function that adjusts the SR of the tunable splitter accordingly after routing
    * The adjustment is done based on the max/min losses of the subtrees thus far
    * 
    * @param level_nodes The top nodes of the current level of the tree
    * @param power_min_max The power map which maps the points of the previous level to min/max power losses in dB
    */

    unordered_map<point, vector<double>, pointHash> re_power_min_max;
    nary_tree_node* subnode = nullptr;
    point key = { -1, -1 };
    double power_inter = 0;

    // For each of the current level nodes
    for (const auto& nodes : level_nodes) {
        // Check if node is a tunable splitter
        if (nodes->dev->object_id == 7) {
            // If yes then pass through each of the subnodes
            for (int sub = 0; sub < 2; sub++) {
                subnode = nodes->children[sub];
                // For each point in power min max
                auto it = power_min_max.find(subnode->dev->print_loc1());
                while (it == power_min_max.end()) {
                    subnode = subnode->children[0];
                    power_inter -= subnode->dev->calc_loss();
                    it = power_min_max.find(subnode->dev->print_loc1());
                }
                re_power_min_max[subnode->dev->print_loc1()] = { it->second[0] + power_inter, it->second[1] + power_inter };
                power_inter = 0;
            }

            double pmax_a = 0;
            double pmax_b = 0;
            double pmin_a = 0;
            double pmin_b = 0;
            vector<point> value_points;

            for (const auto& prev_points : re_power_min_max) {
                value_points.push_back(prev_points.first);
            }

            pmin_a = re_power_min_max[value_points[0]][0];
            pmax_a = re_power_min_max[value_points[0]][1];
            pmin_b = re_power_min_max[value_points[1]][0];
            pmax_b = re_power_min_max[value_points[1]][1];
            double a = (pmax_a + (-pmax_a + pmin_a) / 2) - (pmax_b + (-pmax_b + pmin_b) / 2);
            double a_pow = pow(10, a / 10);
            double SR = 1 / (1 + a_pow);
            if (debugger) {
                cout << pmax_a << " " << pmin_a << " " << pmax_b << " " << pmin_b << endl;
                cout << "Setting new SR = " << SR << endl;
            }
            char label_identifier = nodes->dev->label[20];
            // Dynamic Casting to access the object of tunable splitter
            tunable_splitter1x2* ts1x2 = dynamic_cast<tunable_splitter1x2*>(nodes->dev);
            try {
                if (ts1x2) {
                    ts1x2->set_sr(SR);
                    ts1x2->set_path_sr1(value_points[0]);
                    ts1x2->set_path_sr2(value_points[1]);
                    nodes->dev->label = "Tunable Splitter1x2_" + to_string(label_identifier) + " SR = " + to_string(SR) +
                        " at (" + to_string(ts1x2->get_path_sr1().x_coord) + ", " + to_string(ts1x2->get_path_sr1().y_coord) + ")";
                }
                else {
                    throw runtime_error("Could not access tunable splitter 1x2!\n");
                }
            }
            catch (const runtime_error& error) {
                setTextColor(RED);
                cerr << "Error: " << error.what() << endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void splitterReplacer(unordered_map<point, vector<double>, pointHash> powers_per_point, vector<nary_tree_node*>& tops, unordered_map<point, vector<point>, pointHash> groups, int label_identifier) {
    /**
    * This code checks if there is a power violation for the current level of the tree
    * If yes, then it replaces a 1x2 splitter with a tunable 1x2 splitter
    * If the node is not a 1x2 splitter, but a 1x3 or 1x5, then it throws an error
    * @param powers_per_point A map that connects the min/max powers to a certain point of the subtree
    * @param tops The current top nodes of the subtrees constructed thus far
    * @param groups A map that connects each group to their children 
    * @param label_identifier The new name for the tunable 1x2 splitter if there is a violation
    */
    // Check for each subtree if there is a power violation
    if (debugger) {
        setTextColor(YELLOW);
        cout << "Splitter Replacer commences..." << endl;
        setTextColor(WHITE);
    }

    vector<point> violated_points;
    double SR = -1;
    vector<nary_tree_node*> extra_tops;

    // for each top node of the subtrees
    for (const auto& pts : tops) {
        // find the grouped points that correspond to it
        vector<point> vec_locs = groups[pts->dev->print_loc1()];
        vector<double> power_mins;
        vector<double> power_maxs;
        unordered_map<point, double, pointHash> child_inter_power;
        // calculate the intermediate power till it reaches the children
        for (const auto& ch : pts->children) {
            point next_loc = ch->children[0]->dev->print_loc1();
            nary_tree_node* next = ch->children[0];
            double power_inter = 0;
            while (find(vec_locs.begin(), vec_locs.end(), next_loc) != vec_locs.end()) {
                power_inter += next->dev->calc_loss();
                next = next->children[0];
                next_loc = next->dev->print_loc1();
            }
            child_inter_power[next_loc] = power_inter;
        }
        // for those points find the min/max power values and save them in vectors
        for (const auto& loc : vec_locs) {
            double power_min = powers_per_point[loc][0] + child_inter_power[loc];
            double power_max = powers_per_point[loc][1] + child_inter_power[loc];
            power_mins.push_back(power_min);
            power_maxs.push_back(power_max);
        }
        // find the minimum of all mins and maximum of all maxs
        double min1 = *min_element(power_mins.begin(), power_mins.end());
        double max1 = *max_element(power_maxs.begin(), power_maxs.end());
        double power_diff = max1 - min1;

        // check if there is a violation and replace the 1x2 splitter with a tunable one
        // support of replacing 1x3 and 1x5 with tunable 1x2 to be added
        try {
            if (power_diff > power_thres && vec_locs.size() == 2) {
                if (debugger) {
                    setTextColor(YELLOW);
                    cerr << "Power Violation detected at splitter 1x2...\n";
                    cerr << "Trying to fix the violation...\n";
                    setTextColor(WHITE);
                }

                // calculate the SR value
                double pmax_a = power_maxs[0];
                double pmax_b = power_maxs[1];
                double pmin_a = power_mins[0];
                double pmin_b = power_mins[1];
                double a = (pmax_a + (-pmax_a + pmin_a) / 2) - (pmax_b + (-pmax_b + pmin_b) / 2);
                double a_pow = pow(10, a / 10);
                SR = 1 / (1 + a_pow);
                vector<nary_tree_node*> old_children = pts->children;
                point xy_start = pts->dev->print_loc1();
                try {
                    auto it = find(tops.begin(), tops.end(), pts);
                    if (it != tops.end()) {
                        tops.erase(it);
                    }
                    else {
                        throw runtime_error("Could not find element to delete!\n");
                    }
                }
                catch (const runtime_error& error) {
                    setTextColor(RED);
                    cerr << "Error: " << error.what() << endl;
                    exit(EXIT_FAILURE);
                }

                tunable_splitter1x2* sp_new = new tunable_splitter1x2(xy_start, 7, "Tunable Splitter1x2_" + to_string(label_identifier) + " SR = " + to_string(SR));
                sp_new->set_sr(SR);
                sp_new->set_path_sr1(vec_locs[0]);
                sp_new->set_path_sr2(vec_locs[1]);
                nary_tree_node* new_top = new nary_tree_node{ sp_new, old_children };
                extra_tops.push_back(new_top);
                if (debugger) {
                    setTextColor(GREEN);
                    cout << "Tunable splitter replacement of 1x2 complete!\n";
                    setTextColor(WHITE);
                }
            }
            else if (power_diff > power_thres && vec_locs.size() == 3) {
                throw runtime_error("Power Violation detected at splitter 1x3...\n");
            }
            else if (power_diff > power_thres && vec_locs.size() == 5) {
                throw runtime_error("Power Violation detected at splitter 1x5...\n");
            }
        }
        catch (const runtime_error& error) {
            setTextColor(RED);
            cerr << "Error: " << error.what() << endl;
            exit(EXIT_FAILURE);
        }
    }
      
    if (debugger) {
        setTextColor(GREEN);
        cout << "Splitter Replacer finished!\n";
        setTextColor(WHITE);
    }
}

// -----------------------------------------------------------------------------------------------------------------------

//! deprecated code for overlap detection
//! 
/*
// 2
// Second DFS for overlap detection
bool DFS_overlap(nary_tree_node* node, photo_device* wg) {

    //cout << "STARTS OVERLAP CHECKS" << endl;

    // In case the node corresponds to a waveguide
    if (node->dev->object_id == 1) {
        point xy_start = node->dev->print_loc1();
        point xy_end = node->dev->print_loc2();
        // Checks for overlaps
        bool detect = overlap_detection(xy_start, xy_end, wg->print_loc1(), wg->print_loc2());
        // Returns true if it finds an overlap
        if (detect) {
            //cout << "- FOUND AN OVERLAP" << endl;
            return true;
        }
    }

    // Returns false if no children present
    if (node->children.size() == 0 || node == NULL) {
        //cout << "EXITS" << endl;
        return false;
    }

    vector<bool> all_subtrees;

    // Repeat for searching each child
    for (int i = 0; i < node->children.size(); i++) {
        all_subtrees.push_back(DFS_overlap(node->children[i], wg));
    }

    // Returns true if it finds at least one overlap
    return accumulate(all_subtrees.begin(), all_subtrees.end(), false,
        [](bool a, bool b) { return a || b; });
}


// Overlap top function, gets the node tree and the waveguide to be tested
bool top_DFS_overlap(vector<nary_tree_node*> tops, photo_device* wg) {

    vector<bool> all_overlap_checks;

    // For each subtree check for overlaps
    for (int i = 0; i < tops.size(); i++) {
        all_overlap_checks.push_back(DFS_overlap(tops[i], wg));
    }

    // Return the accumulated result
    bool result = accumulate(all_overlap_checks.begin(), all_overlap_checks.end(), false,
        [](bool a, bool b) { return a || b; });

    return result;
}


// -----------------------------------------------------------------------------------------------------------------------
vector<line> checkHorSameY(vector<line>& horizontal, double y) {

vector<line> aligned_lines;

for (const auto& lines : horizontal) {
    if (lines.p1.y_coord == y) {
        aligned_lines.push_back(lines);
    }
}

return aligned_lines;
}

// -----------------------------------------------------------------------------------------------------------------------
vector<line> checkVertSameX(vector<line>& vertical, double x) {

    vector<line> aligned_lines;

    for (const auto& lines : vertical) {
        if (lines.p1.x_coord == x) {
            aligned_lines.push_back(lines);
        }
    }

    return aligned_lines;
}

// -----------------------------------------------------------------------------------------------------------------------
vector<line> checkHorPassX(vector<line>& horizontal, double x, double y, point p1, point p2) {

    vector<line> passed_lines;

    for (const auto& lines : horizontal) {
        double hoz_y = lines.p1.y_coord;
        if (((lines.p1.x_coord >= x && lines.p2.x_coord <= x) || (lines.p2.x_coord >= x && lines.p1.x_coord <= x))
            && ((p2.y_coord > p1.y_coord && y > hoz_y) || (p2.y_coord < p1.y_coord && y < hoz_y))) {
            passed_lines.push_back(lines);
        }
    }

    return passed_lines;
}

vector<line> checkVertPassY(vector<line>& vertical, double y, double x, point p1, point p2) {

    vector<line> passed_lines;

    // First check is to find the vertical lines that have a specific y inbetween them 
    // Second check is to find the vertical lines that have not been passed by yet
    for (const auto& lines : vertical) {
        double vert_x = lines.p1.x_coord;
        if (((lines.p1.y_coord >= y && lines.p2.y_coord <= y) || (lines.p2.y_coord >= y && lines.p1.y_coord <= y))
            && ((p2.x_coord > p1.x_coord && x > vert_x) || (p2.x_coord < p1.x_coord && x < vert_x))) {
            passed_lines.push_back(lines);
            //cout << "Pass" << endl;
        }
    }

    return passed_lines;
}


struct top_cases {
    vector<line> connection;
    point end_point;
    double power_cost;
    bool passed;
    vector<line> hoz;
    vector<line> vert;
};


bool compareXLines(const line& a, const line& b) {
    return max(a.p1.x_coord, a.p2.x_coord) < max(b.p1.x_coord, b.p1.x_coord);
}

bool compareYLines(const line& a, const line& b) {
    return max(a.p1.y_coord, a.p2.y_coord) < max(b.p1.y_coord, b.p1.y_coord);
}

*/


