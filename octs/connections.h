// Top optical routing functions
// -----------------------------------------------------------------------------------------------------------------------

unordered_map<point, int, pointHash> findVerticalOrHorizontal(unordered_map<int, vector<point>> new_groups) {
    /**
    * Function to find the direction of the routing based on the concentration of the new group
    * The direction can either start horizontally or vertically
    * This controls if there will be crosses or not
    * Essentially if the X distance is larger we route horizontally, but if Y distance is larger we route vertically
    * 
    * @param new_groups The newly grouped MPs from post-linear optimization grouping
    */
    unordered_map<point, int, pointHash> verts_or_hozs;
    double x;
    double y;

    // Find the estimated middle points for each new group of MPs
    for (const auto& pair : new_groups) {

        x = 0.0;
        y = 0.0;

        // Find middle point for each new group
        for (const auto& pointy : pair.second) {
            x += pointy.x_coord;
            y += pointy.y_coord;
        }
        x /= pair.second.size();
        y /= pair.second.size();

        point middle = { x, y };

        int index = 0;

        // Check relative location of each point to the middle 
        for (const auto& pointy : pair.second) {
            double diff_x = pointy.x_coord - middle.x_coord;
            double diff_y = pointy.y_coord - middle.y_coord;

            if (abs(diff_x) > abs(diff_y)) {
                verts_or_hozs[pointy] = 0; // 1 means vertical
            }
            else {
                verts_or_hozs[pointy] = 1; // 0 means horizontal
            }
        }
    }

    return verts_or_hozs;
}

// -----------------------------------------------------------------------------------------------------------------------

tuple<vector<nary_tree_node*>, vector<int>> create_connections(unordered_map<int, point> new_points,
    unordered_map<int, vector<point>> groups, vector<nary_tree_node*> previous_level_top, vector<int> comp_nums, unordered_map<int, int> rotations,
    unordered_map<point, vector<int>, pointHash> point_dev_map, unordered_map<point, double, pointHash> SRs,
    unordered_map<int, vector<point>> new_groups) {
    /**
    * Top function used for optical routing
    * First step consists of creating the respective splitter based on the MP
    * Second step connects the splitter with the respective MP
    * 
    * @param new_points The locations of the MPs
    * @param groups The points corresponding to each group 
    * @param previous_leve_top The previous top N-ary nodes defined from previous routings
    * @param comp_nums A vector used for counting the number of components
    * @param rotations The rotations of 1x3 and 1x5 splitter if applicable
    * @param point_dev_map Map that connects a top point of a subtree to a vector of integers 
    that corresponds to the number of components for that subtree
    * @param SRs The splitting ratios for 1x2 splitters if applicable
    * @param new_groups The new groups found from post-linear optimization grouping
    */

    vector<nary_tree_node*> this_level_top;
    int number_of_variables = static_cast<int>(groups.size());
    try{
        // Routing for each group
        for (int label = 0; label < number_of_variables; label++) {

            // Get the points for the respective group
            vector<point> prev_level = groups[label];

            // This is the merging point for the respective group
            point top_point = new_points[label];

            unordered_map<point, point, pointHash> mapper_point_input; // Necessary for 1x3 and 1x5 splitters
            nary_tree_node* new_top = nullptr; // Initialize new_top node as a null pointer
            nary_tree_node* splitter_top = nullptr; // Initialize splitter_top node as a null pointer
            nary_tree_node* top_wave_node = nullptr; // waveguide used for 1x3 and 1x5 splitters

            // Initialize the pointers to the respective objects
            splitter1x2* sp = new splitter1x2();
            splitter1x3* s13 = new splitter1x3();
            splitter1x5* s15 = new splitter1x5();
            tunable_splitter1x2* spt = new tunable_splitter1x2();

            // Setting the top splitters for each scenario
            switch (groups[label].size()) {
            case 1: {
                // For 1 MP we pass this loop
                continue;
            }
                  break;
            case 2: {
                // Check Number of MPs and either create a normal splitter or a tunable one, depending on the number of sinks of the subtree
                point p1 = groups[label][0];
                point p2 = groups[label][1];
                point split = new_points[label];
                if (point_dev_map[p1][3] != point_dev_map[p2][3]) {
                    spt = new tunable_splitter1x2(top_point, 7, "Tunable Splitter1x2_" + to_string(comp_nums[7]) + " SR = " + to_string(SRs[split]));
                    comp_nums[7]++;
                    new_top = set_new_node(spt);
                    previous_level_top.push_back(new_top);
                    delete sp;
                    if (debugger) {
                        setTextColor(GREEN);
                        cout << "Created 1x2 splitter." << endl;
                        setTextColor(WHITE);
                    }
                }
                else {
                    sp = new splitter1x2(top_point, 2, "Splitter1x2_" + to_string(comp_nums[2]));
                    comp_nums[2]++;
                    new_top = set_new_node(sp);
                    previous_level_top.push_back(new_top);
                    delete spt;
                    if (debugger) {
                        setTextColor(GREEN);
                        cout << "Created tunable 1x2 splitter." << endl;
                        setTextColor(WHITE);
                    }
                }
                delete s13;
                delete s15;
            }
                  break;
            case 3: {
                point wave_edge;
                point splitter_loc;
                switch (rotations[label]) {
                case 0:
                    wave_edge = { top_point.x_coord, top_point.y_coord - length_spl1x3 - length_wg_spl1x3 };
                    splitter_loc = { top_point.x_coord, top_point.y_coord - length_spl1x3 };
                    break;
                case 90:
                    wave_edge = { top_point.x_coord - length_spl1x3 - length_wg_spl1x3, top_point.y_coord };
                    splitter_loc = { top_point.x_coord - length_spl1x3, top_point.y_coord };
                    break;
                case 180:
                    wave_edge = { top_point.x_coord, top_point.y_coord + length_spl1x3 + length_wg_spl1x3 };
                    splitter_loc = { top_point.x_coord, top_point.y_coord + length_spl1x3 };
                    break;
                case 270:
                    wave_edge = { top_point.x_coord + length_spl1x3 + length_wg_spl1x3, top_point.y_coord };
                    splitter_loc = { top_point.x_coord + length_spl1x3, top_point.y_coord };
                    break;
                default:
                    throw runtime_error("Invalid rotation option for splitter 1x3!\n");
                }
                wave_edge = roundPointToPrecision(wave_edge, precision);
                splitter_loc = roundPointToPrecision(splitter_loc, precision);
                s13 = new splitter1x3(splitter_loc, 5, "Splitter1x3_" + to_string(comp_nums[5]), 2 * width_spl1x3, length_spl1x3 + length_wg_spl1x3, rotations[label]);
                comp_nums[5]++;
                mapper_point_input = mapConnectionInputs(rotations[label], groups[label], new_points[label]);
                splitter_top = set_new_node(s13);
                waveguide* wg_top = new waveguide(wave_edge, splitter_loc, 1, "Net : F ign");
                top_wave_node = set_new_node(wg_top);
                connect_trees(top_wave_node, splitter_top);
                previous_level_top.push_back(top_wave_node);
                delete sp;
                delete s15;
                delete spt;
                if (debugger) {
                    setTextColor(GREEN);
                    cout << "Created 1x3 splitter." << endl;
                    setTextColor(WHITE);
                }
            }
                  break;
            case 5: {
                point wave_edge;
                point splitter_loc;
                switch (rotations[label]) {
                case 0:
                    wave_edge = { top_point.x_coord, top_point.y_coord - length_spl1x5 - length_wg_spl1x5 };
                    splitter_loc = { top_point.x_coord, top_point.y_coord - length_spl1x5 };
                    break;
                case 90:
                    wave_edge = { top_point.x_coord - length_spl1x5 - length_wg_spl1x5, top_point.y_coord };
                    splitter_loc = { top_point.x_coord - length_spl1x5, top_point.y_coord };
                    break;
                case 180:
                    wave_edge = { top_point.x_coord, top_point.y_coord + length_spl1x5 + length_wg_spl1x5 };
                    splitter_loc = { top_point.x_coord, top_point.y_coord + length_spl1x5 };
                    break;
                case 270:
                    wave_edge = { top_point.x_coord + length_spl1x5 + length_wg_spl1x5, top_point.y_coord };
                    splitter_loc = { top_point.x_coord + length_spl1x5, top_point.y_coord };
                    break;
                default:
                    throw runtime_error("Invalid rotation option for splitter 1x5!\n");
                }
                wave_edge = roundPointToPrecision(wave_edge, precision);
                splitter_loc = roundPointToPrecision(splitter_loc, precision);
                s15 = new splitter1x5(splitter_loc, 6, "Splitter1x5_" + to_string(comp_nums[6]), 2 * width_spl1x5, length_spl1x5 + length_wg_spl1x5, rotations[label]);
                comp_nums[6]++;
                mapper_point_input =
                    mapConnectionInputs(rotations[label], groups[label], new_points[label]);
                splitter_top = set_new_node(s15);
                waveguide* wg_top = new waveguide(wave_edge, splitter_loc, 1, "Net : L ign");
                top_wave_node = set_new_node(wg_top);
                connect_trees(top_wave_node, splitter_top);
                previous_level_top.push_back(top_wave_node);
                delete sp;
                delete s13;
                delete spt;
                if (debugger) {
                    setTextColor(GREEN);
                    cout << "Created 1x5 splitter." << endl;
                    setTextColor(WHITE);
                }
            }
                  break;
            default:
                throw runtime_error("Cannot create splitter for this number of points!\n");
            }

            nary_tree_node* previous_node;
            vector<point> sorted_points;

            // Sort the inputs - point routing based on the map, for 1x3 and 1x5 splitters - sorting from edge inputs to central one
            // This tactic is necessary to detect overlaps as we move on towards the center of the splitter
            if (groups[label].size() != 2) {
                double width = 0;
                if (groups[label].size() == 5) {
                    width = width_spl1x5;
                }
                else if (groups[label].size() == 3){
                    width = width_spl1x3;
                }
                for (const auto& pair : mapper_point_input) {
                    point this_point = pair.second;
                    if ((rotations[label] == 0 || rotations[label] == 180) && abs(this_point.x_coord) == width) {
                        sorted_points.push_back(pair.first);
                    }
                    else if ((rotations[label] == 90 || rotations[label] == 270) && abs(this_point.y_coord) == width) {
                        sorted_points.push_back(pair.first);
                    }
                }
                if (groups[label].size() == 5) {
                    for (const auto& pair : mapper_point_input) {
                        point this_point = pair.second;
                        if ((rotations[label] == 0 || rotations[label] == 180) && abs(this_point.x_coord) == width_spl1x5 / 2) {
                            sorted_points.push_back(pair.first);
                        }
                        else if ((rotations[label] == 90 || rotations[label] == 270) && abs(this_point.y_coord) == width_spl1x5 / 2) {
                            sorted_points.push_back(pair.first);
                        }
                    }
                }
                for (const auto& pair : mapper_point_input) {
                    point this_point = pair.second;
                    if ((rotations[label] == 0 || rotations[label] == 180) && abs(this_point.x_coord) == 0) {
                        sorted_points.push_back(pair.first);
                    }
                    else if ((rotations[label] == 90 || rotations[label] == 270) && abs(this_point.y_coord) == 0) {
                        sorted_points.push_back(pair.first);
                    }
                }
                prev_level = sorted_points;
            }

            // Route each point in the subtree 
            int dir = -1;
            for (int subtree = 0; subtree < prev_level.size(); subtree++) {

                // Get last destination
                point point_final = prev_level[subtree];

                // Adjustment point
                point adj_point = point(-1, -1);

                // Find the adjustment points
                switch (groups[label].size()) {
                case 2:
                    break;
                case 3: {
                    point adj = mapper_point_input[prev_level[subtree]];
                    pair<nary_tree_node*, point> pairy = createSplitter1x3Input(splitter_top, adj, subtree, rotations[label]);
                    new_top = pairy.first;
                    top_point = pairy.second;
                }
                      break;
                case 5: {
                    point adj = mapper_point_input[prev_level[subtree]];
                    pair<nary_tree_node*, point> pairy = createSplitter1x5Input(splitter_top, adj, subtree, rotations[label]);
                    new_top = pairy.first;
                    top_point = pairy.second;
                }
                      break;
                default:
                    throw runtime_error("Cannot create splitter for this number of points.\n");
                }

                unordered_map<point, int, pointHash> verts_or_hozs = findVerticalOrHorizontal(new_groups);
                tuple<vector<line>, vector<point>> tuple_lines;
                if (subtree == 0) {
                    tuple_lines = traverseGraph(previous_level_top, top_point, point_final, verts_or_hozs[top_point]);
                    dir = verts_or_hozs[top_point];
                }
                else {
                    tuple_lines = traverseGraph(previous_level_top, top_point, point_final, dir);
                }

                vector<line> connections = get<0>(tuple_lines);
                reverse(connections.begin(), connections.end());
                vector<point> crosses = get<1>(tuple_lines);

                previous_node = new_top;

                // Implement the connections, bends and crosses
                bool dir_finished = true; // checks if it finished routing towards a direction
                point first = { -1, -1 };
                point temp_second = { -1, -1 };
                bool first_time = true;

                for (const auto& conns : connections) {
                
                    // Save the first point of the line
                    if (dir_finished) {
                        first = conns.p2;
                        temp_second = conns.p1;
                        dir_finished = false;
                    }

                    if ((conns.p1.x_coord == conns.p2.x_coord && first.x_coord == temp_second.x_coord) ||
                        (conns.p1.y_coord == conns.p2.y_coord && first.y_coord == temp_second.y_coord)) {
                        temp_second = conns.p1;
                    }
                    else {
                        waveguide* wg = new waveguide(first, temp_second, 1, "Net : " + to_string(comp_nums[1]));
                        //cout << "Waveguide created starting at " << first << " and ending at " << temp_second << "in bend" << endl;
                        add_child(previous_node, wg);
                        // Previous node is a splitter
                        if (first_time && (previous_node->dev->object_id == 2 || previous_node->dev->object_id == 5 ||
                            previous_node->dev->object_id == 6 || previous_node->dev->object_id == 7)) {
                            previous_node = previous_node->children[subtree];
                            first_time = false;
                        }
                        else {
                            // Previous node has a single child because it is either waveguide/cross/bend
                            previous_node = previous_node->children[0];
                        }
                        comp_nums[1]++; // Increase count for waveguides
                        // Create a bend
                        bend* bd = new bend(temp_second, 0, "Bend : " + to_string(comp_nums[0]));
                        if (debugger) {
                            cout << "Bend created at " << temp_second << endl;
                            cout << previous_node->dev->label << endl;
                        }
                        add_child(previous_node, bd);
                        previous_node = previous_node->children[0];
                        comp_nums[0]++;
                        first = temp_second;
                        temp_second = conns.p1;
                    }

                    // Cross detection 
                    for (const auto& pt : crosses) {
                        if ((pt.x_coord == first.x_coord && pt.y_coord > min(first.y_coord, temp_second.y_coord) && pt.y_coord < max(first.y_coord, temp_second.y_coord)) ||
                            (pt.y_coord == first.y_coord && pt.x_coord > min(first.x_coord, temp_second.x_coord) && pt.x_coord < max(first.x_coord, temp_second.x_coord))) {
                            waveguide* wg = new waveguide(first, pt, 1, "Net : " + to_string(comp_nums[1]));
                            add_child(previous_node, wg);
                            if (first_time && (previous_node->dev->object_id == 2 || previous_node->dev->object_id == 5 ||
                                previous_node->dev->object_id == 6 || previous_node->dev->object_id == 7)) {
                                previous_node = previous_node->children[subtree];
                                first_time = false;
                            }
                            else {
                                previous_node = previous_node->children[0];
                            }
                            cross* cr = new cross(pt, 3, "Cross : " + to_string(comp_nums[3]));
                            add_child(previous_node, cr);
                            previous_node = previous_node->children[0];
                            comp_nums[3]++; // Increase cross object count
                            first = pt;
                            temp_second = conns.p1;
                        }
                    }
                }
                waveguide* wg = new waveguide(first, temp_second, 1, "Net : " + to_string(comp_nums[1]));

                add_child(previous_node, wg);
                if (first_time && (previous_node->dev->object_id == 2 || previous_node->dev->object_id == 5 || 
                    previous_node->dev->object_id == 6 || previous_node->dev->object_id == 7)) {
                    previous_node = previous_node->children[subtree];
                }
                else {
                    previous_node = previous_node->children[0];
                }

                bool x_st1 = false;
                bool x_st2 = false;
                bool y_st1 = false;
                bool y_st2 = false;

                // Just be careful to make it aligned previous_level_top and run through loop
                size_t found_index = numeric_limits<size_t>::max();
    ;
                for (size_t i = 0; i < previous_level_top.size(); ++i) {
                    if (previous_level_top[i]->dev->print_loc1() == previous_node->dev->print_loc2()) {
                        found_index = i;
                        // In case we find waveguide - waveguide for 1x3 and 1x5 we might need to make a bend
                        // Be careful this is for the cases where 1x3 and 1x5 are children so they have been added
                        // Basically it is because of the short waveguide at the end of the splitters
                        if (previous_level_top[i]->dev->print_loc1().x_coord == previous_level_top[i]->dev->print_loc2().x_coord && previous_level_top[i]->dev->object_id == 1) {
                            x_st1 = true;
                        }
                        else if (previous_level_top[i]->dev->print_loc1().y_coord == previous_level_top[i]->dev->print_loc2().y_coord && previous_level_top[i]->dev->object_id == 1) {
                            y_st1 = true;
                        }
                        if (previous_node->dev->print_loc1().x_coord == previous_node->dev->print_loc2().x_coord && previous_node->dev->object_id == 1) {
                            x_st2 = true;
                        }
                        else if (previous_node->dev->print_loc1().y_coord == previous_node->dev->print_loc2().y_coord && previous_node->dev->object_id == 1) {
                            y_st2 = true;
                        }
                        if (((x_st1 && y_st2) || (x_st2 && y_st1))) {
                            bend* bd_final = new bend(previous_node->dev->print_loc2(), 0, "Bend : " + to_string(comp_nums[0]));
                            comp_nums[0]++;
                            add_child(previous_node, bd_final);
                            previous_node = previous_node->children[0];
                        }
                        if (debugger) {
                            setTextColor(GREEN);
                            cout << "Trees got merged!\n";
                            setTextColor(WHITE);
                        }
                        break;
                    }
                }
                if (found_index == numeric_limits<size_t>::max()) {
                    throw runtime_error("Value exceeds the limit for tree merging! The tree root could not be found!\n");
                }
                connect_trees(previous_node, previous_level_top[found_index]);
                // Searches for any crosses, because they have to be defined for the previous levels too
                top_DFS_cross(previous_level_top, crosses);
            }

            // The new top level is either the splitter or the short waveguide depending on if the splitter is 1x2 or 1x3/1x5
            if (groups[label].size() == 2) {
                this_level_top.push_back(new_top);
            }
            else {
                this_level_top.push_back(top_wave_node);
            }
        }
    }
    catch (const runtime_error& error) {
        setTextColor(RED);
        cerr << "Error: " << error.what() << endl;
        exit(EXIT_FAILURE);
    }

    return make_tuple(this_level_top, comp_nums);
}

// -----------------------------------------------------------------------------------------------------------------------

//! Deprecated Code
/*
if (is_there_overlap || is_there_overlap2 || overlap_A) {
    // Routing second type, horizontal then vertical
    point_inter = { point_final.x_coord , top_point.y_coord };
    wg1 = new waveguide(top_point, point_inter, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;
    wg2 = new waveguide(point_inter, point_final, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;

    // For checking purposes
    is_there_overlap = top_DFS_overlap(previous_level_top, wg1);
    is_there_overlap2 = top_DFS_overlap(previous_level_top, wg2);

    // Routing third type should do vertical then horizontal
    if (is_there_overlap) {
        cout << "Overlap abort" << endl;
        cout << "Abort No1 " << endl;
        abort();
    }
}*/

// Sort the cross points accordingly, unless we have no crosses
/*cout << "Cross size = " << all_crosses.size() << endl;
if (all_crosses.size() == 0) {
    cout << "Adds the child" << endl;
    add_child(previous_node, wg1);
    previous_node = previous_node->children[subtree];
}
else {
    sort(all_crosses.begin(), all_crosses.end(), compare_points);
    point previous_point = top_point;
    int index = -1;
    for (int i = 0; i < all_crosses.size(); i++) {
        if ((top_point.x_coord == all_crosses[0].x_coord && top_point.y_coord > all_crosses[0].y_coord)
            || (top_point.y_coord == all_crosses[0].y_coord && top_point.x_coord > all_crosses[0].x_coord)) {
            index = all_crosses.size() - 1 - i;
        }
        else {
            index = i;
        }
        waveguide* wg_inter = new waveguide(previous_point, all_crosses[index], 1, "Net : " + to_string(comp_nums[1]));
        comp_nums[1]++;
        cout << "Created waveguide" << endl;
        add_child(previous_node, wg_inter);
        size_t this_size = previous_node->children.size();
        cross* crs = new cross(all_crosses[index], 3, "Cross : " + to_string(comp_nums[3]));
        comp_nums[3]++;
        cout << "Created cross" << endl;
        add_child(previous_node->children[this_size - 1], crs);
        previous_point = all_crosses[index];
        previous_node = previous_node->children[this_size - 1]->children[0];
        last_cross_or_splitter = previous_node;
    }
    waveguide* wg_final = new waveguide(previous_point, point_inter, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;
    cout << "Created waveguide" << endl;
    add_child(previous_node, wg_final);
    previous_node = previous_node->children[0];
}*/


// Overlap testing No 2 
/*if (is_there_overlap || overlap_A) {
    point_inter = { point_final.x_coord , top_point.y_coord };
    wg2 = new waveguide(point_inter, point_final, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;
    cout << "Created waveguide" << endl;
    // For checking purposes
    bool is_there_overlap2 = top_DFS_overlap(previous_level_top, wg2);
    if (is_there_overlap2) {
        cout << "Overlap abort?" << endl;
        tuple<vector<int>, nary_tree_node*> my_tuple = overlap_avoidance_typeB(last_cross_or_splitter,
            point_final, point_inter, comp_nums, previous_level_top, subtree);
        comp_nums = get<0>(my_tuple);
        previous_node = get<1>(my_tuple);
        check = true;
    }
}*/


/*if (all_crosses.size() == 0) {
    cout << "Add child" << endl;
    add_child(previous_node, wg2);
}
else {
    sort(all_crosses.begin(), all_crosses.end(), compare_points);
    point previous_point = point_inter;
    int index = -1;
    for (int i = 0; i < all_crosses.size(); i++) {
        if ((point_inter.x_coord == all_crosses[0].x_coord && point_inter.y_coord > all_crosses[0].y_coord)
            || (point_inter.y_coord == all_crosses[0].y_coord && point_inter.x_coord > all_crosses[0].x_coord)) {
            index = all_crosses.size() - 1 - i;
        }
        else {
            index = i;
        }
        waveguide* wg_inter = new waveguide(previous_point, all_crosses[index], 1, "Net : " + to_string(comp_nums[1]));
        comp_nums[1]++;
        cout << "Created waveguide" << endl;
        add_child(previous_node, wg_inter);
        size_t this_size = previous_node->children.size();
        cross* crs = new cross(all_crosses[index], 3, "Cross : " + to_string(comp_nums[3]));
        comp_nums[3]++;
        cout << "Created cross" << endl;
        add_child(previous_node->children[this_size - 1], crs);
        previous_point = all_crosses[index];
        previous_node = previous_node->children[this_size - 1]->children[0];
    }
    waveguide* wg_final = new waveguide(previous_point, point_final, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;
    cout << "Created waveguide" << endl;
    add_child(previous_node, wg_final);
}*/




/*
if (is_there_overlap || is_there_overlap2 || overlap_A) {
    // Routing second type, horizontal then vertical
    point_inter = { point_final.x_coord , top_point.y_coord };
    wg1 = new waveguide(top_point, point_inter, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;
    wg2 = new waveguide(point_inter, point_final, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;

    // For checking purposes
    is_there_overlap = top_DFS_overlap(previous_level_top, wg1);
    is_there_overlap2 = top_DFS_overlap(previous_level_top, wg2);

    // Routing third type should do vertical then horizontal
    if (is_there_overlap) {
        cout << "Overlap abort" << endl;
        cout << "Abort No1 " << endl;
        abort();
    }
}*/



// Sort the cross points accordingly, unless we have no crosses
/*cout << "Cross size = " << all_crosses.size() << endl;
if (all_crosses.size() == 0) {
    cout << "Adds the child" << endl;
    add_child(previous_node, wg1);
    previous_node = previous_node->children[subtree];
}
else {
    sort(all_crosses.begin(), all_crosses.end(), compare_points);
    point previous_point = top_point;
    int index = -1;
    for (int i = 0; i < all_crosses.size(); i++) {
        if ((top_point.x_coord == all_crosses[0].x_coord && top_point.y_coord > all_crosses[0].y_coord)
            || (top_point.y_coord == all_crosses[0].y_coord && top_point.x_coord > all_crosses[0].x_coord)) {
            index = all_crosses.size() - 1 - i;
        }
        else {
            index = i;
        }
        waveguide* wg_inter = new waveguide(previous_point, all_crosses[index], 1, "Net : " + to_string(comp_nums[1]));
        comp_nums[1]++;
        cout << "Created waveguide" << endl;
        add_child(previous_node, wg_inter);
        size_t this_size = previous_node->children.size();
        cross* crs = new cross(all_crosses[index], 3, "Cross : " + to_string(comp_nums[3]));
        comp_nums[3]++;
        cout << "Created cross" << endl;
        add_child(previous_node->children[this_size - 1], crs);
        previous_point = all_crosses[index];
        previous_node = previous_node->children[this_size - 1]->children[0];
        last_cross_or_splitter = previous_node;
    }
    waveguide* wg_final = new waveguide(previous_point, point_inter, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;
    cout << "Created waveguide" << endl;
    add_child(previous_node, wg_final);
    previous_node = previous_node->children[0];
}*/


// Overlap testing No 2 
/*if (is_there_overlap || overlap_A) {
    point_inter = { point_final.x_coord , top_point.y_coord };
    wg2 = new waveguide(point_inter, point_final, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;
    cout << "Created waveguide" << endl;
    // For checking purposes
    bool is_there_overlap2 = top_DFS_overlap(previous_level_top, wg2);
    if (is_there_overlap2) {
        cout << "Overlap abort?" << endl;
        tuple<vector<int>, nary_tree_node*> my_tuple = overlap_avoidance_typeB(last_cross_or_splitter,
            point_final, point_inter, comp_nums, previous_level_top, subtree);
        comp_nums = get<0>(my_tuple);
        previous_node = get<1>(my_tuple);
        check = true;
    }
}*/


/*if (all_crosses.size() == 0) {
    cout << "Add child" << endl;
    add_child(previous_node, wg2);
}
else {
    sort(all_crosses.begin(), all_crosses.end(), compare_points);
    point previous_point = point_inter;
    int index = -1;
    for (int i = 0; i < all_crosses.size(); i++) {
        if ((point_inter.x_coord == all_crosses[0].x_coord && point_inter.y_coord > all_crosses[0].y_coord)
            || (point_inter.y_coord == all_crosses[0].y_coord && point_inter.x_coord > all_crosses[0].x_coord)) {
            index = all_crosses.size() - 1 - i;
        }
        else {
            index = i;
        }
        waveguide* wg_inter = new waveguide(previous_point, all_crosses[index], 1, "Net : " + to_string(comp_nums[1]));
        comp_nums[1]++;
        cout << "Created waveguide" << endl;
        add_child(previous_node, wg_inter);
        size_t this_size = previous_node->children.size();
        cross* crs = new cross(all_crosses[index], 3, "Cross : " + to_string(comp_nums[3]));
        comp_nums[3]++;
        cout << "Created cross" << endl;
        add_child(previous_node->children[this_size - 1], crs);
        previous_point = all_crosses[index];
        previous_node = previous_node->children[this_size - 1]->children[0];
    }
    waveguide* wg_final = new waveguide(previous_point, point_final, 1, "Net : " + to_string(comp_nums[1]));
    comp_nums[1]++;
    cout << "Created waveguide" << endl;
    add_child(previous_node, wg_final);
}*/