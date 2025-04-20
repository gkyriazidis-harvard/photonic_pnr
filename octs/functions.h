#pragma once

// -----------------------------------------------------------------------------------------------------------------------

struct nary_tree_node {
    /**
     * N-ary node structure
     * Used to create the tree structure
     * @param dev Corresponding photonic device
     * @param children Connections with other N-ary nodes
     */
    photo_device* dev;
    vector<nary_tree_node*> children;
};

// -----------------------------------------------------------------------------------------------------------------------

struct nary_tree_node* set_new_node(photo_device* dev) {
    /**
     * Function that creates a node for the N-ary tree
     * Does not create connections with children or parent nodes
     * Mainly used for creating the sink nodes
     * @param dev Photonic device corresponding to that specific node
     * @param new_node N-ary node structure
     */
    nary_tree_node* new_node = new nary_tree_node();
    new_node->dev = dev;
    return new_node;
}

// -----------------------------------------------------------------------------------------------------------------------

void add_child(nary_tree_node* node, photo_device* dev) {
    /**
     * Function that creates an N-ary node with a photonic device then adds this node as a child of another node
     * @param node The parent node
     * @param dev The photonic device to be used at the child node
     */
    nary_tree_node* new_node = set_new_node(dev);
    node->children.push_back(new_node);
}

// -----------------------------------------------------------------------------------------------------------------------

void connect_trees(nary_tree_node* node_parent, nary_tree_node* node_child) {
    /**
     * Function that connects two N-ary nodes together keeping all their connections intact
     * @param node_parent The parent node
     * @param node_child The child node to be connected
     */
    node_parent->children.push_back(node_child);
}

// -----------------------------------------------------------------------------------------------------------------------

void remove_child_cross(nary_tree_node* parent_node, nary_tree_node* child_node, point cross_pt) {
    /**
     * Function that severes a connection between a parent node and its child to create a cross node and two new nodes
     * It then re-establishes the connection between the nodes with the cross node in the middle
     * @param parent_node The parent node whose connection will be split
     * @param child_node The child node where cross appears. Always corresponds to a waveguide.
     * @param cross_pt which corresponds to the location of the cross
     */
    for (int i = 0; i < parent_node->children.size(); i++) {
        if (parent_node->children[i]->dev->label == child_node->dev->label) {
            vector<nary_tree_node*> old_children = child_node->children;
            string old_label = child_node->dev->label;
            point xy_start = child_node->dev->print_loc1();
            point xy_end = child_node->dev->print_loc2();

            // Traverse the vector and remove the target node
            for (auto it = parent_node->children.begin(); it != parent_node->children.end(); ++it) {
                if (*it == child_node) {
                    parent_node->children.erase(it);
                    break;
                }
            }

            // Instantiate new objects
            waveguide* wg1 = new waveguide(xy_start, cross_pt, 1, old_label + "A");
            cross* crs = new cross(cross_pt, 3, "Cross");
            waveguide* wg2 = new waveguide(cross_pt, xy_end, 1, old_label + "B");

            // Renew connections
            add_child(parent_node, wg1);
            size_t this_size = parent_node->children.size();
            add_child(parent_node->children[this_size - 1], crs);
            add_child(parent_node->children[this_size - 1]->children[0], wg2);
            // Reconnect the trees together
            parent_node->children[this_size - 1]->children[0]->children[0]->children = old_children;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------------------

//! Depricated not used any more
point find_common_point(point line1_start, point line1_end, point line2_start, point line2_end) {
    double x1, y1, x2, y2, x3, y3, x4, y4;
    x1 = line1_start.x_coord;
    y1 = line1_start.y_coord;
    x2 = line1_end.x_coord;
    y2 = line1_end.y_coord;
    x3 = line2_start.x_coord;
    y3 = line2_start.y_coord;
    x4 = line2_end.x_coord;
    y4 = line2_end.y_coord;
    double intersection_x = -1;
    double intersection_y = -1;
    if (x1 == x2 && y3 == y4 && x1 != x3 && y1 != y3 && x1 != x4 && y2 != y3) {
        intersection_x = x1;
        intersection_y = y3;
    }
    else if (x3 == x4 && y1 == y2 && x1 != x3 && y1 != y3 && x2 != x3 && y1 != y4) {
        intersection_x = x3;
        intersection_y = y1;
    }
    else if (x1 == x2 && x3 == x4) {
        point p1{ -1,-1 };
        return p1;
    }
    else if (y1 == y2 && y3 == y4) {
        point p1{ -1,-1 };
        return p1;
    }
    if (min(x1, x2) <= intersection_x && intersection_x <= max(x1, x2) &&
        min(y1, y2) <= intersection_y && intersection_y <= max(y1, y2) &&
        min(x3, x4) <= intersection_x && intersection_x <= max(x3, x4) &&
        min(y3, y4) <= intersection_y && intersection_y <= max(y3, y4)) {
        point p1{ intersection_x, intersection_y };
        return p1;
    }
    point p1{ -1,-1 };
    return p1;
}

// -----------------------------------------------------------------------------------------------------------------------

double manhattan_distance(point point1, point point2) {
    /**
     * Calculates the manhattan distance between two points
     * @param point1 First point
     * @param point2 Second point
     * @param distance Distance between the two points in mm
     */

    double distance = abs(point1.x_coord - point2.x_coord) + abs(point1.y_coord - point2.y_coord);
    return distance;
}

// -----------------------------------------------------------------------------------------------------------------------

vector<point> extract_coordinates(const string& input_filename) {
    /**
     * This function reads the sink locations from a received file
     * * The coordinates of each sink should be noted after the word "Coordinates" and should be in um
     * @param input_filename String parameter that corresponds to the name of the file
     * @param output Returns a vector of points that is the locations of the sinks in (x,y) format
     */

    ifstream file(input_filename);

    vector<double> posx;
    vector<double> posy;
    string line;
    vector<point> output;

    if (file.is_open()) {
        vector<string> lines;
        string line;

        // Read all lines from the file
        while (getline(file, line)) {
            lines.push_back(line);
        }

        // Start from line 14 and iterate in steps of 4 until the end of the file
        for (int i = 0; i < lines.size(); i ++) {
            line = lines[i];
            istringstream iss(line);
            string discard;
            string coordinates_str;

            getline(iss, discard, ':');
            string search_word = "Coordinates";
            size_t found = discard.find(search_word);

            if (found != string::npos){
                // Extract the coordinates part after ':'
                getline(iss, coordinates_str);

                istringstream coordinates_ss(coordinates_str);
                double x, y;

                // Extract the X and Y coordinates and transform them into mm
                if (coordinates_ss >> x >> y) {
                    output.push_back({ x / 1000.0 , y / 1000.0});

                }
            }
        }

        file.close();
    }
    else {
        cerr << "Failed to open the file: " << input_filename << endl;
    }

    return output;
}

// -----------------------------------------------------------------------------------------------------------------------

void setTextColor(int color) {
    /**
    * Used to add colors in the command line texts
    */
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// -----------------------------------------------------------------------------------------------------------------------

vector<int> prime_factorization(int n) {
    /**
     * This function is responsible for prime factors of an integer number
     * @param n This is the number two analyze into prime factors
     * @param factors A vector consisting of the prime factors of number n
     */
    vector<int> factors;
    if (n <= 1) return factors; 

    while (n % 2 == 0) {
      factors.push_back(2);
      n /= 2;
    }

    for (int i = 3; i <= sqrt(n); i += 2) {
      while (n % i == 0) {
        factors.push_back(i);
        n /= i;
      }
    }
  
    if (n > 2) {
      factors.push_back(n);
    }
  
    return factors;
  }

// -----------------------------------------------------------------------------------------------------------------------

double calculate_power_from_SR_1x2(double SR){
    /**
     * Function that calculates the power consumption from a heater element
     * * This function works only for heaters on 1x2 splitters
     * @param SR The splitting ratio, value between 0 and 1
     * @param power The power consumption of the heater for a specific SR
     */

    // Parameters to define the relationship between power and SR
    double a = 0;
    double b = 5.0 / 6.0;
    double c = -5.0 / 6.0;
    double d = 0.2;

    double power = abs(a * pow(SR, 3) + b * pow(SR, 2) + c * pow(SR, 1) + d);

    return power;
}

// -----------------------------------------------------------------------------------------------------------------------

bool are_all_elements_identical(const vector<int>& vec) {
    /**
     * This function checks if all the elements of a vector are identical
     * @param vec The vector to check its elements
     * @param value The output which is True if all elements are identical or False otherwise
     */
    if (vec.empty()) {
      return true; 
    }

    bool value = adjacent_find(vec.begin(), vec.end(), not_equal_to<>()) == vec.end();
  
    return value;
  }

// -----------------------------------------------------------------------------------------------------------------------

double estimate_group_power(vector<int> groups){
    /**
     * Function that creates an estimation of the power consumption based on the grouped sink number
     * * This function must be adjusted if other splitters support heaters (so far only 1x2 are assumed to support heaters)
     * @param groups The number of sinks for each subgroup in the main group vector
     * @param cost The power consumption value for the specific group
     */

    double cost = 0.0;

    if (groups.size() == 2 && !are_all_elements_identical(groups)){
        double me = *max_element(groups.begin(), groups.end());
        double tot = accumulate(groups.begin(), groups.end(), 0);
        double SR = me / tot;
        cost = calculate_power_from_SR_1x2(SR);
    } else if (groups.size() > 2 && !are_all_elements_identical(groups)){
        cerr << "Heaters are not supported for other splitters!" << endl;
    }

    return cost;
}

// -----------------------------------------------------------------------------------------------------------------------

void cout_maps(unordered_map<point, vector<int>, pointHash> point_dev_map,
    unordered_map<point, vector<double>, pointHash> dist_min_max,
    unordered_map<point, vector<double>, pointHash> power_min_max) {
    /**
     * Function that is used for debugging purposes
     * Prints all the information about the current clock tree iteration saved in the maps
     * Information include the max/min power in dB consumed from each subtree, the max/min delays for each subtree and the number of elements for each subtree
     * @param point_dev_map Map that contains the devices for each subtree
     * @param dist_min_max Map that contains the max/min distances -> delays for each subtree
     * @param power_min_max Map that contains the max/min powers for each subtree
     */
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

// -----------------------------------------------------------------------------------------------------------------------

// Function to generate all combinations of a specific size, gets one vector of points
//! Deprecated
vector<vector<point>> generate_combinations_1vec(const vector<point>& points, int size) {
    vector<vector<point>> combinations;
    vector<bool> mask(points.size() - size, false);
    mask.insert(mask.end(), size, true);

    do {
        vector<point> combination;
        for (size_t i = 0; i < points.size(); ++i) {
            if (mask[i]) {
                combination.push_back(points[i]);
            }
        }
        combinations.push_back(combination);
    } while (next_permutation(mask.begin(), mask.end()));

    return combinations;
}

void remove_duplicates(vector<int>& vec) {
    /**
    * Function that removes duplicates from a vector of integers
    * @param vec The vector of integers
    */
    sort(vec.begin(), vec.end());

    auto it = unique(vec.begin(), vec.end());

    vec.erase(it, vec.end());
}

// Function to generate combinations of 2 between two vectors
//! Deprecated
vector<vector<point>> generate_combinations_2vec(const vector<point>& vector1, const vector<point>& vector2) {
    vector<vector<point>> combinations;

    for (const auto& point1 : vector1) {
        for (const auto& point2 : vector2) {
            vector<point> combination;
            combination.push_back(point1);
            combination.push_back(point2);
            combinations.push_back(combination);
        }
    }

    return combinations;
}

// Function that searches the nearby nodes for overlap
//! Deprecated
bool overlap_detection(point line1_start, point line1_end, point line2_start, point line2_end) {
    double x1, y1, x2, y2, x3, y3, x4, y4;
    x1 = line1_start.x_coord;
    y1 = line1_start.y_coord;
    x2 = line1_end.x_coord;
    y2 = line1_end.y_coord;
    x3 = line2_start.x_coord;
    y3 = line2_start.y_coord;
    x4 = line2_end.x_coord;
    y4 = line2_end.y_coord;

    if (x1 == x2 && x3 == x4 && x2 == x3) {
        if (!(y1 <= y3 && y2 <= y3 && y1 <= y4 && y2 <= y4) &&
            !(y1 >= y3 && y2 >= y3 && y1 >= y4 && y2 >= y4)) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (y1 == y2 && y3 == y4 && y2 == y3) {
        if (!(x1 <= x3 && x2 <= x3 && x1 <= x4 && x2 <= x4) &&
            !(x1 >= x3 && x2 >= x3 && x1 >= x4 && x2 >= x4)) {
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

// Compare two points on the same line
//! Deprecated
bool compare_points(const point& p1, const point& p2) {
    if (p1.x_coord != p2.x_coord) {
        return p1.x_coord < p2.x_coord; // Sort by x if x coordinates are not constant
    }
    else {
        return p1.y_coord < p2.y_coord; // Sort by y if x coordinates are constant
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void iterTreePrinter(const nary_tree_node* top, ofstream& output_file) {
    /**
     * Iterative function that prints all the components with their location and distances starting from the top node
     * @param top The top node of the N-ary tree that has been constructed
     * @param output_file The name of the txt file
     */

    if (top == nullptr) {
        return;
    }

    // Waveguides
    if (top->dev->object_id == 1) {
        output_file << top->dev->label << endl;
        output_file << "\tCoordinate : " <<
            top->dev->print_loc1().x_coord * 1000 << " " << top->dev->print_loc1().y_coord * 1000 << " " <<
            top->dev->print_loc2().x_coord * 1000 << " " << top->dev->print_loc2().y_coord * 1000 << endl;
    } // Bends, Crosses and Photodetectors
    else if (top->dev->object_id == 0 || top->dev->object_id == 3 || top->dev->object_id == 4){
        output_file << top->dev->label << endl;
        output_file << "\tCoordinate : " <<
            top->dev->print_loc1().x_coord * 1000 << " " << top->dev->print_loc1().y_coord * 1000 << endl;
    } // 1x2, 1x3, 1x5 and Tunable 1x2 Splitters
    else if (top->dev->object_id == 2 || top->dev->object_id == 5 || top->dev->object_id == 6 || top->dev->object_id == 7) {
        output_file << "Merge point - " << top->dev->label << endl;
        output_file << "\tCoordinate : " <<
            top->dev->print_loc1().x_coord * 1000 << " " << top->dev->print_loc1().y_coord * 1000 << endl;
    }

    for (int i = 0; i < top->children.size(); i++) {
        iterTreePrinter(top->children[i], output_file);
    }
}

// -----------------------------------------------------------------------------------------------------------------------
// Variables needed to count the number of sinks and merging points to correctly print things in the verilog file

void iterTreePrinterVerilog(const nary_tree_node* top, ofstream& output_file, vector<string> prev_conns, int this_i) {
    /**
     * Iterative function that passes through all the nodes of the N-ary tree to define the splitter instances and the assigns
     * @param top The top node of the N-ary tree that has been constructed
     * @param output_file The name of the verilog file
     * @param prev_conns The outputs of the previous splitter component used
     * @param num The number identifying the corresponding splitter
     * @param i The number identifying the corresponding output of a splitter
     * @param num_sinks The number identifying a specific sink
     */

    if (top == nullptr) {
        return;
    }

    string prev_conn;

    // 1x2, 1x3, 1x5 and Tunable 1x2 Splitters
    if (top->dev->object_id == 2) {
        string num_mps = top->dev->label.substr(12);
        if (prev_conns[0] == "source"){
            output_file << endl;
            output_file << "\t assign nx2_" << num_mps << "_i = source;" << endl;
            prev_conn = "nx2_" + num_mps + "_i";
        } else {
            prev_conn = prev_conns[this_i];
        }
        prev_conns.clear();
        output_file << endl;
        output_file << "\t Splitter1x2 inst" << num_mps << "(.in(" << prev_conn << "), .out_0(nx2_" <<  \
            num_mps << "_o[0]), .out_1(nx2_" << num_mps << "_o[1]));" << endl;
        prev_conns.push_back("nx2_" + num_mps + "_o[0]");
        prev_conns.push_back("nx2_" + num_mps + "_o[1]");
    }
    if (top->dev->object_id == 5) {
        string num_mps = top->dev->label.substr(12);
        if (prev_conns[0] == "source"){
            output_file << endl;
            output_file << "\t assign nx3_" << num_mps << "_i = source;" << endl;
            prev_conn = "nx3_" + num_mps + "_i";
        } else {
            prev_conn = prev_conns[this_i];
        }
        prev_conns.clear();
        output_file << endl;
        output_file << "\t Splitter1x3 inst" << num_mps << "(.in(" << prev_conn << "), .out_0(nx3_" <<  \
            num_mps << "_o[0]), .out_1(nx3_" << num_mps << "_o[1]), .out_2(nx3_" << num_mps << "_o[2]));" << endl;
        prev_conns.push_back("nx3_" + num_mps + "_o[0]");
        prev_conns.push_back("nx3_" + num_mps + "_o[1]");
        prev_conns.push_back("nx3_" + num_mps + "_o[2]");
    }
    if (top->dev->object_id == 6) {
        string num_mps = top->dev->label.substr(12);
        if (prev_conns[0] == "source"){
            output_file << endl;
            output_file << "\t assign nx5_" << num_mps << "_i = source;" << endl;
            prev_conn = "nx5_" + num_mps + "_i";
        } else {
            prev_conn = prev_conns[this_i];
        }
        prev_conns.clear();
        output_file << endl;
        output_file << "\t Splitter1x5 inst" << num_mps << "(.in(" << prev_conn << "), .out_0(nx5_" <<  \
            num_mps << "_o[0]), .out_1(nx5_" << num_mps << "_o[1]), .out_2(nx5_" << num_mps << "_o[2]), .out_3(nx5_" << \
            num_mps << "_o[3]), .out_4(nx5_" << num_mps << "_o[4]));" << endl;
        prev_conns.push_back("nx5_" + num_mps + "_o[0]");
        prev_conns.push_back("nx5_" + num_mps + "_o[1]");
        prev_conns.push_back("nx5_" + num_mps + "_o[2]");
        prev_conns.push_back("nx5_" + num_mps + "_o[3]");
        prev_conns.push_back("nx5_" + num_mps + "_o[4]");
    }
    if (top->dev->object_id == 7) {
        string num_mps = top->dev->label.substr(20, top->dev->label.length() - 33);
        if (prev_conns[0] == "source"){
            output_file << endl;
            output_file << "\t assign nx2dc_" << num_mps << "_i = source;" << endl;
            prev_conn = "nx2dc_" + num_mps + "_i";
        } else {
            prev_conn = prev_conns[this_i];
        }
        prev_conns.clear();
        output_file << endl;
        output_file << "\t DCHeater2x2 inst" << num_mps << "(.in_0(), .in_1(" << prev_conn << "), .out_0(nx2dc_" <<  \
            num_mps << "_o[0]), .out_1(nx2dc_" << num_mps << "_o[1]));" << endl;
        prev_conns.push_back("nx2dc_" + num_mps + "_o[0]");
        prev_conns.push_back("nx2dc_" + num_mps + "_o[1]");
    }

    if (top->dev->object_id == 4){
        prev_conn = prev_conns[this_i];
        output_file << endl;
        output_file << "\t assign sinks[" << top->dev->label.substr(7) << "] = " << prev_conn << ";" << endl;
    }

    for (int i = 0; i < top->children.size(); i++) {
        if (top->dev->object_id == 7 || top->dev->object_id == 6 || top->dev->object_id == 5 || top->dev->object_id == 2) {
            this_i = i;
        }
        iterTreePrinterVerilog(top->children[i], output_file, prev_conns, this_i);
    }

    // Free up memory
    delete top->dev;
}

// -----------------------------------------------------------------------------------------------------------------------

void fullTreePrinter(const nary_tree_node* top, ofstream& output_file, vector<int> comp_nums) {
    /**
     * A function that generates the clock tree based on a specific format. 
     * @param top The top node of the N-ary tree that has been constructed
     * @param output_file The name of the verilog file
     * @param comp_nums A vector of integers showing the total amount of each of the components used
     */

    output_file << "# Generated by OCTS." << endl;
    output_file << endl;
    output_file << "NumSinks : " << comp_nums[4] << endl;
    output_file << "NumBends : " << comp_nums[0] << endl;
    output_file << "NumCrosses : " << comp_nums[3] << endl;
    output_file << "NumSplitters1x2 : " << comp_nums[2] << endl;
    output_file << "NumTunableSplitters1x2 : " << comp_nums[7] << endl;
    output_file << "NumSplitters1x3 : " << comp_nums[5] << endl;
    output_file << "NumSplitters1x5 : " << comp_nums[6] << endl;
    output_file << endl;
    output_file << "Source : " << top->dev->print_loc1().x_coord * 1000 << " " << top->dev->print_loc1().y_coord * 1000 << endl;
    output_file << endl;
    iterTreePrinter(top, output_file);
    return;
}

// -----------------------------------------------------------------------------------------------------------------------

void verilogCreator(const nary_tree_node* top, ofstream& output_file, vector<int> comp_nums){
    /**
     * Function that produces the Verilog output file with solely splitters and directional couplers based on the N-ary tree
     * @param top The top node of the N-ary tree that has been constructed
     * @param output_file The name of the verilog file
     * @param comp_nums A vector of integers showing the total amount of each of the components used
     */
    output_file << "module top_clock(" << endl;
    output_file << "\t input source," << endl;
    output_file << "\t output [" << comp_nums[4] - 1 << ":0] sinks" << endl;
    output_file << ");" << endl;
    output_file << endl;
    output_file << "\t //Defining the wires" << endl;
    output_file << endl;
    switch (top->dev->object_id) {
    case 2: 
        output_file << "\t wire nx2_0_i;" << endl;
        break;
    case 5:
        output_file << "\t wire nx3_0_i;" << endl;
        break;
    case 6: 
        output_file << "\t wire nx5_0_i;" << endl;
        break;
    case 7:
        output_file << "\t wire nx2dc_0_i;" << endl;
        break;
    }

    for (int i = 0; i < comp_nums[2]; i++){
        output_file << "\t wire [1:0] nx2_" << i << "_o;" << endl;
    }
    for (int i = 0; i < comp_nums[5]; i++){
        output_file << "\t wire [2:0] nx3_" << i << "_o;" << endl;
    }
    for (int i = 0; i < comp_nums[6]; i++){
        output_file << "\t wire [4:0] nx5_" << i << "_o;" << endl;
    }
    for (int i = 0; i < comp_nums[7]; i++){
        output_file << "\t wire [1:0] nx2dc_" << i << "_o;" << endl;
    }
    output_file << "\t wire [" << comp_nums[4] - 1 << ":0] sinks;" << endl;

    output_file << endl;
    output_file << "\t //Creating the instances and assigning to sinks and source" << endl;
    iterTreePrinterVerilog(top, output_file, {"source"}, 0);
    output_file << endl;
    output_file << "endmodule" << endl;
    return;
}

// -----------------------------------------------------------------------------------------------------------------------

unordered_map<point, vector<int>, pointHash> initialize_map(const vector<point>& points) {
    unordered_map<point, vector<int>, pointHash> point_map;

    vector<int> initial_vector = { 0, 0, 0, 1, 0, 0, 0 };

    for (const auto& item : points) {
        point_map[item] = initial_vector;
    }

    return point_map;
}

// -----------------------------------------------------------------------------------------------------------------------

unordered_map<point, vector<double>, pointHash> initialize_map2(const vector<point>& points) {
    /**
     * Initialization function that initializes the map
     * The map connects each point with a vector of double values corresponding to a min/max value which can be distance or power
     * This map is used to keep track of the power ande delay changes while constructing the clock tree
     * @param points Vector of points used to initialize the map
     * @param point_map Map that connects each point with a vector of two double values
     */
    unordered_map<point, vector<double>, pointHash> point_map;

    vector<double> initial_vector = { 0, 0 };

    for (const auto& item : points) {
        point_map[item] = initial_vector;
    }

    return point_map;
}

// -----------------------------------------------------------------------------------------------------------------------

vector<nary_tree_node*> photo_init(const vector<point>& points) {
    /**
     * Function that creates the sink nodes of the N-ary tree
     * @param points Vector containing the locations of the sinks 
     * @param this_vector Vector with the N-ary nodes corresponding to the sinks
     */
    vector<nary_tree_node*> this_vector;
    nary_tree_node* this_node;
    int num = 0;

    for (const auto& pt : points) {
        photodetector* pd = new photodetector(pt, 4, "Sink : " + to_string(num));
        this_node = set_new_node(pd);
        this_vector.push_back(this_node);
        num++;
    }

    return this_vector;
}

// -----------------------------------------------------------------------------------------------------------------------
// Rounding functions used to round the double variables ot a specific precision 
// This is done to avoid any calculation issues that might appear
double roundToPrecision(double value, int precision) {
    double factor = pow(10, precision);
    return round(value * factor) / factor;
}

point roundPointToPrecision(point value, int precision) {
    double factor = pow(10, precision);
    return point{ round(value.x_coord * factor) / factor, round(value.y_coord * factor) / factor };
}

vector<point> roundAllPoints(vector<point> points, int precision) {
    vector<point> clean_points;
    for (const auto& item : points) {
        clean_points.push_back({ roundToPrecision(item.x_coord, precision), roundToPrecision(item.y_coord, precision) });
    }
    return clean_points;
}

unordered_map<int, point> roundMappedPoints(unordered_map<int, point> points, int precision) {
    unordered_map<int, point> clean_points;
    for (const auto& item : points) {
        clean_points[item.first] = roundPointToPrecision(item.second, precision);
    }
    return clean_points;
}

unordered_map<int, vector<point>> roundMappedVectoredPoints(unordered_map<int, vector<point>> points, int precision) {
    unordered_map<int, vector<point>> clean_points;
    for (const auto& item : points) {
        clean_points[item.first] = roundAllPoints(item.second, precision);
    }
    return clean_points;
}

// -----------------------------------------------------------------------------------------------------------------------
string object_rec(photo_device* pd) {
    /**
    * Function that identifies an object based on the identification number
    * Purely used for debugging purposes
    * 
    * @param pd The photonic device
    */
    string object_name;

    switch (pd->object_id) {
    case 0: object_name = "Bend";
        break;
    case 1: object_name = "Waveguide";
        break;
    case 2: object_name = "Splitter 1x2";
        break;
    case 3: object_name = "Cross";
        break;
    case 4: object_name = "Photodetector";
        break;
    case 5: object_name = "Splitter 1x3";
        break;
    case 6: object_name = "Splitter 1x5";
        break;
    case 7: object_name = "Tunable Splitter 1x2";
        break;
    default: object_name = "No recognizable object.";
        break;
    }

    return object_name;
}
// -----------------------------------------------------------------------------------------------------------------------

// Function to find the rotation of the splitters 1x3 and 1x5
// This function uses the mid point in a new group and check the x/y difference between each group point and the mid point
// If the x difference is higher, 90 or 270 degrees are chosen depending on the sign
// If the y difference is higher, 0 or 180 degrees are chosen depending on the sign
// Rotation must be chosen in a way that it does not obstruct the routing process
// This code supports only rotations 0, 90, 180 and 270, different rotations require non orthogonal waveguides for routing
unordered_map<int, int> findRotationExp(unordered_map<int, vector<point>> new_groups,
    unordered_map<int, vector<point>> groups, unordered_map<int, point> mps) {

    unordered_map<int, int> rotations;
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
        //cout << "MIDDLE POINT : " << middle << endl;

        int index = 0;

        // Check relative location of each point to the middle 
        for (const auto& pointy : pair.second) {
            double diff_x = pointy.x_coord - middle.x_coord;
            double diff_y = pointy.y_coord - middle.y_coord;
            /*for (const auto& groupys : groups) {
                cout << groupys.first << endl;
                cout << groupys.second[0] << endl;
                cout << groupys.second[1] << endl;
                cout << "---" << endl;
                cout << "Pointy:" << pointy << endl;
            }
            cout << "NUM = " << index << endl;*/
            for (const auto& groupy : mps) {
                if (groupy.second == pointy) {
                    index = groupy.first;
                }
            }
            // Support of 1x3 and 1x5 splitters, to be generalized
            if (groups[index].size() == 3 || groups[index].size() == 5) {
                if (abs(diff_x) > abs(diff_y)) {
                    if (diff_x > 0) {
                        rotations[index] = 90; 
                    }
                    else {
                        rotations[index] = 270; 
                    }
                }
                else {
                    if (diff_y > 0) {
                        rotations[index] = 0;
                    }
                    else {
                        rotations[index] = 180;
                    }
                }
            }
            else {
                //cerr << "This splitter is not supported by the code or does not have a specific viable rotation." << endl;
                rotations[index] = -1;
            }
        }
    }

    return rotations;
}

// -----------------------------------------------------------------------------------------------------------------------

bool comparePointsByX(const point& p1, const point& p2) {
    /**
    * Compares points based on their X coordinate
    * @param p1 First point 
    * @param p2 Second point
    */
    return p1.x_coord < p2.x_coord;
}

// -----------------------------------------------------------------------------------------------------------------------

bool comparePointsByY(const point& p1, const point& p2) {
    /**
    * Compares points based on their Y coordinate
    * @param p1 First point
    * @param p2 Second point
    */
    return p1.y_coord < p2.y_coord;
}

// -----------------------------------------------------------------------------------------------------------------------

// Adjustment creation based on the rotation and the group size
//! Deprecated
vector<point> adjustment_creation(int rotation, int group_size) {

    point add1, add2, add3, add4, add5;
    vector<point> adjs;

    if (group_size == 3) {
        switch (rotation) {
        case 0:
            add1 = { 0.075, -0.4 };
            add2 = { 0, -0.4 };
            add3 = { -0.075, -0.4 };
            break;
        case 90:
            add1 = { -0.4, -0.075 };
            add2 = { -0.4, 0 };
            add3 = { -0.4, 0.075 };
            break;
        case 180:
            add1 = { -0.075, 0.4 };
            add2 = { 0, 0.4 };
            add3 = { 0.075, 0.4 };
            break;
        case 270:
            add1 = { 0.4, 0.075 };
            add2 = { 0.4, 0 };
            add3 = { 0.4, -0.075 };
            break;
        }
        adjs.push_back(add1);
        adjs.push_back(add2);
        adjs.push_back(add3);
    }
    else if (group_size == 5) {
        switch (rotation) {
        case 0:
            add1 = { 0.075, -0.4 };
            add2 = { 0.0375, -0.4 };
            add3 = { 0, -0.4 };
            add4 = { -0.0375, -0.4 };
            add5 = { -0.075, -0.4 };
            break;
        case 90:
            add1 = { -0.4, -0.075 };
            add2 = { -0.4, -0.0375 };
            add3 = { -0.4, 0 };
            add4 = { -0.4, 0.0375 };
            add5 = { -0.4, 0.075 };
            break;
        case 180:
            add1 = { -0.075, 0.4 };
            add2 = { -0.0375, 0.4 };
            add3 = { 0, 0.4 };
            add4 = { 0.0375, 0.4 };
            add5 = { 0.075, 0.4 };
            break;
        case 270:
            add1 = { 0.4, 0.075 };
            add2 = { 0.4, 0.0375 };
            add3 = { 0.4, 0 };
            add4 = { 0.4, -0.0375 };
            add5 = { 0.4, -0.075 };
            break;
        }
        adjs.push_back(add1);
        adjs.push_back(add2);
        adjs.push_back(add3);
        adjs.push_back(add4);
        adjs.push_back(add5);
    }
    else {
        cerr << "Error: group size should be either 3 or 5." << endl;
        abort();
    }

    return adjs;
}

// -----------------------------------------------------------------------------------------------------------------------
//! Could check if this function can be reduced in size or adjusted
unordered_map<point, point, pointHash> mapConnectionInputs(int rotation, vector<point> group, point next_point) {
    /**
    * Function that maps the points of a group corresponding to a 1x3 or 1x5 splitter to each of the inputs of the splitter
    * @param rotation The rotation of the splitter in degrees with valid numbers 0, 90, 180, 270
    * @param group The points to be mapped in the vector
    * @param next_point Possible location of the new splitter 
    */

    unordered_map<point, point, pointHash> adjustments;

    point add1, add2, add3, add4, add5;
    // adjustment vector for the inputs
    vector<point> inputs, adjs;
    vector<double> nums;
    try {
        if (group.size() == 3) {
            switch (rotation) {
            case 0:
                add1 = { width_spl1x3, -length_spl1x3 };
                add2 = { 0, -length_spl1x3 };
                add3 = { -width_spl1x3, -length_spl1x3 };
                break;
            case 90:
                add1 = { -length_spl1x3, -width_spl1x3 };
                add2 = { -length_spl1x3, 0 };
                add3 = { -length_spl1x3, width_spl1x3 };
                break;
            case 180:
                add1 = { -width_spl1x3, length_spl1x3 };
                add2 = { 0, length_spl1x3 };
                add3 = { width_spl1x3, length_spl1x3 };
                break;
            case 270:
                add1 = { length_spl1x3, width_spl1x3 };
                add2 = { length_spl1x3, 0 };
                add3 = { length_spl1x3, -width_spl1x3 };
                break;
            }

            adjs.push_back(add1);
            adjs.push_back(add2);
            adjs.push_back(add3);

            switch (rotation) {
            case 0: {
                // Find which points are left and which are right the splitter in the 2D plane
                vector<point> left, right;
                for (int i = 0; i < group.size(); i++) {
                    if (group[i].x_coord >= next_point.x_coord) {
                        right.push_back(group[i]);
                    }
                    else {
                        left.push_back(group[i]);
                    }
                }
                // Group up the leftmost with the left point, or the rightmost with the right point if there is only one
                if (left.size() == 1) {
                    adjustments[left[0]] = adjs[0];
                }
                else if (right.size() == 1) {
                    adjustments[right[0]] = adjs[2];
                }
                // If there are 2 or more  on the other side then we check if they are below or above the splitter
                if (left.size() == 2) {
                    vector<point> above, below;
                    for (int i = 0; i < left.size(); i++) {
                        if (left[i].y_coord >= next_point.y_coord) {
                            above.push_back(left[i]);
                        }
                        else {
                            below.push_back(left[i]);
                        }
                    }
                    if (below.size() == 2) {
                        sort(below.begin(), below.end(), comparePointsByX);
                        adjustments[below[0]] = adjs[1];
                        adjustments[below[1]] = adjs[0];
                    }
                    else if (above.size() == 2) {
                        sort(above.begin(), above.end(), comparePointsByY);
                        adjustments[above[0]] = adjs[0];
                        adjustments[above[1]] = adjs[1];
                    }
                    else if (above.size() == 1) {
                        adjustments[below[0]] = adjs[0];
                        adjustments[above[0]] = adjs[1];
                    }
                }
                else if (right.size() == 2) {
                    vector<point> above, below;
                    for (int i = 0; i < right.size(); i++) {
                        if (right[i].y_coord >= next_point.y_coord) {
                            above.push_back(right[i]);
                        }
                        else {
                            below.push_back(right[i]);
                        }
                    }
                    if (below.size() == 2) {
                        sort(below.begin(), below.end(), comparePointsByX);
                        adjustments[below[0]] = adjs[2];
                        adjustments[below[1]] = adjs[1];
                    }
                    else if (above.size() == 2) {
                        sort(above.begin(), above.end(), comparePointsByY);
                        adjustments[above[0]] = adjs[2];
                        adjustments[above[1]] = adjs[1];
                    }
                    else if (above.size() == 1) {
                        adjustments[below[0]] = adjs[2];
                        adjustments[above[0]] = adjs[1];
                    }
                }
                // If all three of them are on the other side
                if (left.size() == 3) {
                    throw("Three on left side for 1x3 scenario, 0 degrees.");
                }
                else if (right.size() == 3) {
                    throw("Three on right side for 1x3 scenario, 0 degrees.");
                }
            }
                  break;
            case 180: {
                // Find which points are left and which are right the splitter
                vector<point> left, right;
                for (int i = 0; i < group.size(); i++) {
                    if (group[i].x_coord >= next_point.x_coord) {
                        right.push_back(group[i]);
                    }
                    else {
                        left.push_back(group[i]);
                    }
                }
                // Group up the leftmost with the left point, or the rightmost with the right point if there is only one
                if (left.size() == 1) {
                    adjustments[left[0]] = adjs[2];
                }
                else if (right.size() == 1) {
                    adjustments[right[0]] = adjs[0];
                }
                // If there are 2 or more  on the other side then we check if they are below or above the splitter
                if (left.size() == 2) {
                    vector<point> above, below;
                    for (int i = 0; i < left.size(); i++) {
                        if (left[i].y_coord >= next_point.y_coord) {
                            above.push_back(left[i]);
                        }
                        else {
                            below.push_back(left[i]);
                        }
                    }
                    if (below.size() == 2) {
                        sort(below.begin(), below.end(), comparePointsByX);
                        adjustments[below[0]] = adjs[1];
                        adjustments[below[1]] = adjs[2];
                    }
                    else if (above.size() == 2) {
                        sort(above.begin(), above.end(), comparePointsByX);
                        adjustments[above[0]] = adjs[1];
                        adjustments[above[1]] = adjs[2];
                    }
                    else if (above.size() == 1) {
                        adjustments[below[0]] = adjs[1];
                        adjustments[above[0]] = adjs[2];
                    }
                }
                else if (right.size() == 2) {
                    vector<point> above, below;
                    for (int i = 0; i < right.size(); i++) {
                        if (right[i].y_coord >= next_point.y_coord) {
                            above.push_back(right[i]);
                        }
                        else {
                            below.push_back(right[i]);
                        }
                    }
                    if (below.size() == 2) {
                        sort(below.begin(), below.end(), comparePointsByX);
                        adjustments[below[0]] = adjs[1];
                        adjustments[below[1]] = adjs[0];
                    }
                    else if (above.size() == 2) {
                        sort(above.begin(), above.end(), comparePointsByX);
                        adjustments[above[0]] = adjs[0];
                        adjustments[above[1]] = adjs[1];
                    }
                    else if (above.size() == 1) {
                        adjustments[below[0]] = adjs[1];
                        adjustments[above[0]] = adjs[0];
                    }
                }
                // If all three of them are on the other side
                if (left.size() == 3) {
                    throw("Three on left side for 1x3 scenario, 180 degrees.");
                }
                else if (right.size() == 3) {
                    throw("Three on right side for 1x3 scenario, 180 degrees.");
                }
            }
                    break;
            case 90: {
                // Find which points are left and which are right the splitter
                vector<point> above, below;
                for (int i = 0; i < group.size(); i++) {
                    if (group[i].y_coord >= next_point.y_coord) {
                        above.push_back(group[i]);
                    }
                    else {
                        below.push_back(group[i]);
                    }
                }
                // Group up the leftmost with the left point, or the rightmost with the right point if there is only one
                if (above.size() == 1) {
                    adjustments[above[0]] = adjs[0];
                }
                else if (below.size() == 1) {
                    adjustments[below[0]] = adjs[2];
                }
                // If there are 2 or more  on the other side then we check if they are below or above the splitter
                if (above.size() == 2) {
                    vector<point> left, right;
                    for (int i = 0; i < above.size(); i++) {
                        if (above[i].x_coord >= next_point.x_coord) {
                            right.push_back(above[i]);
                        }
                        else {
                            left.push_back(above[i]);
                        }
                    }
                    if (right.size() == 2) {
                        sort(right.begin(), right.end(), comparePointsByX);
                        adjustments[right[0]] = adjs[0];
                        adjustments[right[1]] = adjs[1];
                    }
                    else if (left.size() == 2) {
                        sort(left.begin(), left.end(), comparePointsByY);
                        adjustments[left[0]] = adjs[0];
                        adjustments[left[1]] = adjs[1];
                    }
                    else if (left.size() == 1) {
                        adjustments[left[0]] = adjs[0];
                        adjustments[right[0]] = adjs[1];
                    }
                }
                else if (below.size() == 2) {
                    vector<point> left, right;
                    for (int i = 0; i < below.size(); i++) {
                        if (below[i].x_coord >= next_point.x_coord) {
                            right.push_back(below[i]);
                        }
                        else {
                            left.push_back(below[i]);
                        }
                    }
                    if (right.size() == 2) {
                        sort(right.begin(), right.end(), comparePointsByX);
                        adjustments[right[0]] = adjs[2];
                        adjustments[right[1]] = adjs[1];
                    }
                    else if (left.size() == 2) {
                        sort(left.begin(), left.end(), comparePointsByY);
                        adjustments[left[0]] = adjs[1];
                        adjustments[left[1]] = adjs[2];
                    }
                    else if (left.size() == 1) {
                        adjustments[left[0]] = adjs[2];
                        adjustments[right[0]] = adjs[1];
                    }
                }
                // If all three of them are on the other side
                if (above.size() == 3) {
                    throw("Three on above side for 1x3 scenario, 90 degrees.");
                }
                else if (below.size() == 3) {
                    throw("Three on below side for 1x3 scenario, 90 degrees.");
                }
            }
                   break;
            case 270: {
                // Find which points are left and which are right the splitter
                vector<point> above, below;
                for (int i = 0; i < group.size(); i++) {
                    if (group[i].y_coord >= next_point.y_coord) {
                        above.push_back(group[i]);
                    }
                    else {
                        below.push_back(group[i]);
                    }
                }
                // Group up the leftmost with the left point, or the rightmost with the right point if there is only one
                if (above.size() == 1) {
                    adjustments[above[0]] = adjs[2];
                }
                else if (below.size() == 1) {
                    adjustments[below[0]] = adjs[0];
                }
                // If there are 2 or more  on the other side then we check if they are below or above the splitter
                if (above.size() == 2) {
                    vector<point> left, right;
                    for (int i = 0; i < above.size(); i++) {
                        if (above[i].x_coord >= next_point.x_coord) {
                            left.push_back(above[i]);
                        }
                        else {
                            right.push_back(above[i]);
                        }
                    }
                    if (right.size() == 2) {
                        sort(right.begin(), right.end(), comparePointsByX);
                        adjustments[right[0]] = adjs[2];
                        adjustments[right[1]] = adjs[1];
                    }
                    else if (left.size() == 2) {
                        sort(left.begin(), left.end(), comparePointsByY);
                        adjustments[left[0]] = adjs[2];
                        adjustments[left[1]] = adjs[1];
                    }
                    else if (left.size() == 1) {
                        adjustments[left[0]] = adjs[2];
                        adjustments[right[0]] = adjs[1];
                    }
                }
                else if (below.size() == 2) {
                    vector<point> left, right;
                    for (int i = 0; i < below.size(); i++) {
                        if (below[i].x_coord >= next_point.x_coord) {
                            left.push_back(below[i]);
                        }
                        else {
                            right.push_back(below[i]);
                        }
                    }
                    if (right.size() == 2) {
                        sort(right.begin(), right.end(), comparePointsByX);
                        adjustments[right[0]] = adjs[0];
                        adjustments[right[1]] = adjs[1];
                    }
                    else if (left.size() == 2) {
                        sort(left.begin(), left.end(), comparePointsByY);
                        adjustments[left[0]] = adjs[1];
                        adjustments[left[1]] = adjs[0];
                    }
                    else if (left.size() == 1) {
                        adjustments[left[0]] = adjs[0];
                        adjustments[right[0]] = adjs[1];
                    }
                }
                // If all three of them are on the other side
                if (above.size() == 3) {
                    throw("Three on above side for 1x3 scenario, 90 degrees.");
                }
                else if (below.size() == 3) {
                    throw("Three on below side for 1x3 scenario, 90 degrees.");
                }
            }
                    break;
            default:
                throw("Splitter 1x3, invalid rotation!");
            }
        }
        // FOR 1X5 SPLITTERS
        else if (group.size() == 5) {
            switch (rotation) {
            case 0:
                add1 = { width_spl1x5, -length_spl1x5 };
                add2 = { width_spl1x5 / 2, -length_spl1x5 };
                add3 = { 0, -length_spl1x5 };
                add4 = { -width_spl1x5 / 2, -length_spl1x5 };
                add5 = { -width_spl1x5, -length_spl1x5 };
                break;
            case 90:
                add1 = { -length_spl1x5, -width_spl1x5 };
                add2 = { -length_spl1x5, -width_spl1x5 / 2 };
                add3 = { -length_spl1x5, 0 };
                add4 = { -length_spl1x5, width_spl1x5 / 2 };
                add5 = { -length_spl1x5, width_spl1x5 };
                break;
            case 180:
                add1 = { -width_spl1x5, length_spl1x5 };
                add2 = { -width_spl1x5 / 2, length_spl1x5 };
                add3 = { 0, length_spl1x5 };
                add4 = { width_spl1x5 / 2, length_spl1x5 };
                add5 = { width_spl1x5, length_spl1x5 };
                break;
            case 270:
                add1 = { length_spl1x5, width_spl1x5 };
                add2 = { length_spl1x5, width_spl1x5 / 2 };
                add3 = { length_spl1x5, 0 };
                add4 = { length_spl1x5, -width_spl1x5 / 2 };
                add5 = { length_spl1x5, -width_spl1x5 };
                break;
            }

            adjs.push_back(add1);
            adjs.push_back(add2);
            adjs.push_back(add3);
            adjs.push_back(add4);
            adjs.push_back(add5);

            vector<bool> matched(group.size(), false);

            switch (rotation) {
            case 0: {
                // Find which points are left and which are right the splitter
                vector<point> left, right;
                for (int i = 0; i < group.size(); i++) {
                    if (group[i].x_coord >= next_point.x_coord) {
                        right.push_back(group[i]);
                    }
                    else {
                        left.push_back(group[i]);
                    }
                }
                // Group up the leftmost with the left point, or the rightmost with the right point if there is only one
                if (left.size() == 1) {
                    adjustments[left[0]] = adjs[0];
                }
                if (right.size() == 1) {
                    adjustments[right[0]] = adjs[4];
                }
                // If there are 2 on the other side then we check if they are below or above the splitter
                if (left.size() > 1) {
                    vector<point> above, below;
                    // Find the points above and below the splitter
                    for (int i = 0; i < left.size(); i++) {
                        if (left[i].y_coord >= next_point.y_coord) {
                            above.push_back(left[i]);
                        }
                        else {
                            below.push_back(left[i]);
                        }
                    }
                    int port = 0;

                    // Route the below first by ever increasing ports
                    sort(below.begin(), below.end(), comparePointsByX);
                    int below_size = static_cast<int>(below.size());
                    for (int i = 0; i < below_size; i++) {
                        adjustments[below[below_size - i - 1]] = adjs[port];
                        port++;
                    }

                    // Route the upper next by ever increasing ports
                    sort(above.begin(), above.end(), comparePointsByY);
                    int above_size = static_cast<int>(above.size());
                    for (int i = 0; i < above_size; i++) {
                        adjustments[above[i]] = adjs[port];
                        port++;
                    }
                }
                if (right.size() > 1) {
                    vector<point> above, below;
                    for (int i = 0; i < right.size(); i++) {
                        if (right[i].y_coord >= next_point.y_coord) {
                            above.push_back(right[i]);
                        }
                        else {
                            below.push_back(right[i]);
                        }
                    }
                    int port = 4;

                    // Route the below first by ever increasing ports
                    sort(below.begin(), below.end(), comparePointsByX);
                    int below_size = static_cast<int>(below.size());
                    for (int i = 0; i < below_size; i++) {
                        adjustments[below[i]] = adjs[port];
                        port--;
                    }

                    // Route the upper next by ever increasing ports
                    sort(above.begin(), above.end(), comparePointsByY);
                    int above_size = static_cast<int>(above.size());
                    for (int i = 0; i < above_size; i++) {
                        adjustments[above[i]] = adjs[port];
                        port--;
                    }
                }
            }
                  break;
            case 180: {
                // Find which points are left and which are right the splitter
                vector<point> left, right;
                for (int i = 0; i < group.size(); i++) {
                    if (group[i].x_coord >= next_point.x_coord) {
                        right.push_back(group[i]);
                    }
                    else {
                        left.push_back(group[i]);
                    }
                }
                // Group up the leftmost with the left point, or the rightmost with the right point if there is only one
                if (left.size() == 1) {
                    adjustments[left[0]] = adjs[4];
                }
                if (right.size() == 1) {
                    adjustments[right[0]] = adjs[0];
                }
                // If there are 2 or more  on the other side then we check if they are below or above the splitter
                if (left.size() > 1) {
                    vector<point> above, below;
                    for (int i = 0; i < left.size(); i++) {
                        if (left[i].y_coord >= next_point.y_coord) {
                            above.push_back(left[i]);
                        }
                        else {
                            below.push_back(left[i]);
                        }
                    }
                    int port = 4;

                    // Route the upper next by ever increasing ports
                    sort(above.begin(), above.end(), comparePointsByX);
                    int above_size = static_cast<int>(above.size());
                    for (int i = 0; i < above_size; i++) {
                        adjustments[above[above_size - i - 1]] = adjs[port];
                        port--;
                    }

                    // Route the below first by ever increasing ports
                    sort(below.begin(), below.end(), comparePointsByY);
                    int below_size = static_cast<int>(below.size());
                    for (int i = 0; i < below_size; i++) {
                        adjustments[below[below_size - i - 1]] = adjs[port];
                        port--;
                    }
                }
                if (right.size() > 1) {
                    vector<point> above, below;
                    for (int i = 0; i < right.size(); i++) {
                        if (right[i].y_coord >= next_point.y_coord) {
                            above.push_back(right[i]);
                        }
                        else {
                            below.push_back(right[i]);
                        }
                    }
                    int port = 0;

                    // Route the upper next by ever increasing ports
                    sort(above.begin(), above.end(), comparePointsByX);
                    int above_size = static_cast<int>(above.size());
                    for (int i = 0; i < above_size; i++) {
                        adjustments[above[i]] = adjs[port];
                        port++;
                    }

                    // Route the below first by ever increasing ports
                    sort(below.begin(), below.end(), comparePointsByY);
                    int below_size = static_cast<int>(below.size());
                    for (int i = 0; i < below_size; i++) {
                        adjustments[below[below_size - i - 1]] = adjs[port];
                        port++;
                    }
                }
            }
                    break;
            case 90: {
                // Find which points are left and which are right the splitter
                vector<point> above, below;
                for (int i = 0; i < group.size(); i++) {
                    if (group[i].y_coord >= next_point.y_coord) {
                        above.push_back(group[i]);
                    }
                    else {
                        below.push_back(group[i]);
                    }
                }
                // Group up the leftmost with the left point, or the rightmost with the right point if there is only one
                if (above.size() == 1) {
                    adjustments[above[0]] = adjs[0];
                }
                if (below.size() == 1) {
                    adjustments[below[0]] = adjs[4];
                }
                // If there are 2 or more  on the other side then we check if they are below or above the splitter
                if (above.size() > 1) {
                    vector<point> left, right;
                    for (int i = 0; i < above.size(); i++) {
                        if (above[i].x_coord >= next_point.x_coord) {
                            right.push_back(above[i]);
                        }
                        else {
                            left.push_back(above[i]);
                        }
                    }
                    int port = 0;

                    // Route the upper next by ever increasing ports
                    sort(left.begin(), left.end(), comparePointsByY);
                    int left_size = static_cast<int>(left.size());
                    for (int i = 0; i < left_size; i++) {
                        adjustments[left[i]] = adjs[port];
                        port++;
                    }

                    // Route the below first by ever increasing ports
                    sort(right.begin(), right.end(), comparePointsByX);
                    int right_size = static_cast<int>(right.size());
                    for (int i = 0; i < right_size; i++) {
                        adjustments[right[i]] = adjs[port];
                        port++;
                    }
                }
                if (below.size() > 1) {
                    vector<point> left, right;
                    for (int i = 0; i < below.size(); i++) {
                        if (below[i].x_coord >= next_point.x_coord) {
                            right.push_back(below[i]);
                        }
                        else {
                            left.push_back(below[i]);
                        }
                    }
                    int port = 4;

                    // Route the upper next by ever increasing ports
                    sort(left.begin(), left.end(), comparePointsByY);
                    int left_size = static_cast<int>(left.size());
                    for (int i = 0; i < left_size; i++) {
                        adjustments[left[left_size - i - 1]] = adjs[port];
                        port--;
                    }

                    // Route the below first by ever increasing ports
                    sort(right.begin(), right.end(), comparePointsByX);
                    int right_size = static_cast<int>(right.size());
                    for (int i = 0; i < right_size; i++) {
                        adjustments[right[i]] = adjs[port];
                        port--;
                    }
                }
            }
                   break;
            case 270: {
                // Find which points are left and which are right the splitter
                vector<point> above, below;
                for (int i = 0; i < group.size(); i++) {
                    if (group[i].y_coord >= next_point.y_coord) {
                        above.push_back(group[i]);
                    }
                    else {
                        below.push_back(group[i]);
                    }
                }
                // Group up the leftmost with the left point, or the rightmost with the right point if there is only one
                if (above.size() == 1) {
                    adjustments[above[0]] = adjs[4];
                }
                if (below.size() == 1) {
                    adjustments[below[0]] = adjs[0];
                }
                // If there are 2 or more  on the other side then we check if they are below or above the splitter
                if (above.size() > 1) {
                    vector<point> left, right;
                    for (int i = 0; i < above.size(); i++) {
                        if (above[i].x_coord <= next_point.x_coord) {
                            left.push_back(above[i]);
                        }
                        else {
                            right.push_back(above[i]);
                        }
                    }
                    int port = 4;

                    // Route the below first by ever increasing ports
                    sort(right.begin(), right.end(), comparePointsByY);
                    int right_size = static_cast<int>(right.size());
                    for (int i = 0; i < right_size; i++) {
                        adjustments[right[i]] = adjs[port];
                        port--;
                    }

                    // Route the upper next by ever increasing ports
                    sort(left.begin(), left.end(), comparePointsByX);
                    int left_size = static_cast<int>(left.size());
                    for (int i = 0; i < left_size; i++) {
                        adjustments[left[left_size - i - 1]] = adjs[port];
                        port--;
                    }
                }
                else if (below.size() > 1) {
                    vector<point> left, right;
                    for (int i = 0; i < below.size(); i++) {
                        if (below[i].x_coord <= next_point.x_coord) {
                            left.push_back(below[i]);
                        }
                        else {
                            right.push_back(below[i]);
                        }
                    }
                    int port = 0;

                    // Route the below first by ever increasing ports
                    sort(right.begin(), right.end(), comparePointsByY);
                    int right_size = static_cast<int>(right.size());
                    for (int i = 0; i < right_size; i++) {
                        adjustments[right[right_size - i - 1]] = adjs[port];
                        port++;
                    }

                    // Route the upper next by ever increasing ports
                    sort(left.begin(), left.end(), comparePointsByX);
                    int left_size = static_cast<int>(left.size());
                    for (int i = 0; i < left_size; i++) {
                        adjustments[left[left_size - i - 1]] = adjs[port];
                        port++;
                    }
                }
            }
                    break;
            default:
                throw("Splitter 1x5, invalid rotation.");
                break;
            }
        }
        else {
            throw("Group size should be either 3 or 5.");
        }
    }
    catch (const runtime_error& error) {
        cerr << "Error: " << error.what() << endl;
        exit(EXIT_FAILURE);
    }
    return adjustments;
}

// -----------------------------------------------------------------------------------------------------------------------

pair<nary_tree_node*, point> createSplitter1x3Input(nary_tree_node* splitter_node, point adj, int subtree, int rotation) {
    /**
    * Function that creates the waveguide/bends of the 1x3 Splitter which are ignored in delay and power calculations
    * @param splitter_node The N-ary node corresponding to the 1x3 splitter
    * @param adj The adjustment point
    * @param subtree Number of the subtree
    * @param rotation Rotation of the 1x3 splitter (valid: 0 90 180 270)
    */
    nary_tree_node* return_node;
    point return_point;
    nary_tree_node* previous_node;

    point xy_position = splitter_node->dev->print_loc1();
    double this_x = splitter_node->dev->print_loc1().x_coord;
    double this_y = splitter_node->dev->print_loc1().y_coord;

    point p2, p3, p4, p5, p6, p7;

    // Rotation switch
    switch (rotation) {
    case 0:
        p2 = xy_position;
        p3 = { this_x - width_spl1x3, this_y };
        p4 = { this_x - width_spl1x3, this_y + length_spl1x3 };
        p5 = { this_x, this_y + length_spl1x3 + 0.1 };
        p6 = { this_x + width_spl1x3, this_y };
        p7 = { this_x + width_spl1x3, this_y + length_spl1x3 };
        break;
    case 90:
        p2 = xy_position;
        p3 = { this_x , this_y + width_spl1x3 };
        p4 = { this_x + length_spl1x3 , this_y + width_spl1x3 };
        p5 = { this_x + length_spl1x3 + 0.1 , this_y };
        p6 = { this_x , this_y - width_spl1x3 };
        p7 = { this_x + length_spl1x3 , this_y - width_spl1x3 };
        break;
    case 180:
        p2 = xy_position;
        p3 = { this_x + width_spl1x3 , this_y };
        p4 = { this_x + width_spl1x3 , this_y - length_spl1x3 };
        p5 = { this_x , this_y - length_spl1x3 - 0.1 };
        p6 = { this_x - width_spl1x3 , this_y };
        p7 = { this_x - width_spl1x3 , this_y - length_spl1x3 };
        break;
    case 270:
        p2 = xy_position;
        p3 = { this_x , this_y - width_spl1x3 };
        p4 = { this_x - length_spl1x3 , this_y - width_spl1x3 };
        p5 = { this_x - length_spl1x3 - 0.1 , this_y };
        p6 = { this_x , this_y + width_spl1x3 };
        p7 = { this_x - length_spl1x3 , this_y + width_spl1x3 };
        break;
    default:
        p2 = { -1 , -1 };
        p3 = { -1 , -1 };
        p4 = { -1 , -1 };
        p5 = { -1 , -1 };
        p6 = { -1 , -1 };
        p7 = { -1 , -1 };
        break;
    }

    // Based on adjustment point, make the routing
    // These waveguides do not cross and must not overlap 
    // The splitter is considered to have a specific area that must not overlap with other splitters or overlaps
    // The avoidance of this used area is not considered yet
    if (adj == point{ -width_spl1x3, length_spl1x3 } ||
        adj == point{ length_spl1x3, width_spl1x3 } ||
        adj == point{ width_spl1x3, -length_spl1x3 } ||
        adj == point{ -length_spl1x3, -width_spl1x3 }) {

        // Input 1 used
        waveguide* wg2 = new waveguide(p2, p3, 1, "Net : A ign");
        add_child(splitter_node, wg2);
        previous_node = splitter_node->children[subtree];

        bend* bd1 = new bend(p3, 0, "Bend : A ign");
        add_child(previous_node, bd1);
        previous_node = previous_node->children[0];

        waveguide* wg1 = new waveguide(p3, p4, 1, "Net : B ign");
        add_child(previous_node, wg1);
        return_node = previous_node->children[0];
        return_point = p4;
    }
    else if (adj == point{ 0, length_spl1x3 } ||
        adj == point{ length_spl1x3, 0 } ||
        adj == point{ 0, -length_spl1x3 } ||
        adj == point{ -length_spl1x3, 0 }) {

        // Input 2 used
        waveguide* wg3 = new waveguide(p2, p5, 1, "Net : C ign");
        add_child(splitter_node, wg3);
        return_node = splitter_node->children[subtree];
        return_point = p5;
    }
    else {

        // Input 3 used 
        waveguide* wg4 = new waveguide(p2, p6, 1, "Net : D ign");
        add_child(splitter_node, wg4);
        previous_node = splitter_node->children[subtree];

        bend* bd2 = new bend(p6, 0, "Bend : B ign");
        add_child(previous_node, bd2);
        previous_node = previous_node->children[0];

        waveguide* wg5 = new waveguide(p6, p7, 1, "Net : E ign");
        add_child(previous_node, wg5);
        return_node = previous_node->children[0];
        return_point = p7;
    }

    return make_pair(return_node, return_point);
}

// -----------------------------------------------------------------------------------------------------------------------

pair<nary_tree_node*, point> createSplitter1x5Input(nary_tree_node* splitter_node, point adj, int subtree, int rotation) {
    /**
    * Function that creates the waveguide/bends of the 1x5 Splitter which are ignored in delay and power calculations
    * @param splitter_node The N-ary node corresponding to the 1x3 splitter
    * @param adj The adjustment point
    * @param subtree Number of the subtree
    * @param rotation Rotation of the 1x5 splitter (valid: 0 90 180 270)
    */
    nary_tree_node* return_node;
    point return_point;
    nary_tree_node* previous_node;

    // Put the name
    point xy_position = splitter_node->dev->print_loc1();
    double this_x = splitter_node->dev->print_loc1().x_coord;
    double this_y = splitter_node->dev->print_loc1().y_coord;

    point p2, p3, p4, p5, p6, p7, p8, p9, p10, p11;

    // Rotation switch
    switch (rotation) {
    case 0:
        p2 = xy_position;
        p3 = { this_x - width_spl1x5, this_y };
        p4 = { this_x - width_spl1x5, this_y + length_spl1x5 };
        p5 = { this_x, this_y + length_spl1x5 + 0.2 };
        p6 = { this_x + width_spl1x5, this_y };
        p7 = { this_x + width_spl1x5, this_y + length_spl1x5 };
        p8 = { this_x - width_spl1x5 / 2, this_y };
        p9 = { this_x - width_spl1x5 / 2, this_y + length_spl1x5 + 0.1 };
        p10 = { this_x + width_spl1x5 / 2, this_y };
        p11 = { this_x + width_spl1x5 / 2, this_y + length_spl1x5 + 0.1 };
        break;
    case 90:
        p2 = xy_position;
        p3 = { this_x , this_y + width_spl1x5 };
        p4 = { this_x + length_spl1x5 , this_y + width_spl1x5 };
        p5 = { this_x + length_spl1x5 + 0.2 , this_y };
        p6 = { this_x , this_y - width_spl1x5 };
        p7 = { this_x + length_spl1x5 , this_y - width_spl1x5 };
        p8 = { this_x , this_y + width_spl1x5 / 2 };
        p9 = { this_x + length_spl1x5 + 0.1 , this_y + width_spl1x5 / 2 };
        p10 = { this_x , this_y - width_spl1x5 / 2 };
        p11 = { this_x + length_spl1x5 + 0.1 , this_y - width_spl1x5 / 2 };
        break;
    case 180:
        p2 = xy_position;
        p3 = { this_x + width_spl1x5 , this_y };
        p4 = { this_x + width_spl1x5 , this_y - length_spl1x5 };
        p5 = { this_x , this_y - length_spl1x5 - 0.2 };
        p6 = { this_x - width_spl1x5 , this_y };
        p7 = { this_x - width_spl1x5 , this_y - length_spl1x5 };
        p8 = { this_x + width_spl1x5 / 2 , this_y };
        p9 = { this_x + width_spl1x5 / 2, this_y - length_spl1x5 - 0.1 };
        p10 = { this_x - width_spl1x5 / 2 , this_y };
        p11 = { this_x - width_spl1x5 / 2 , this_y - length_spl1x5 - 0.1 };
        break;
    case 270:
        p2 = xy_position;
        p3 = { this_x , this_y - width_spl1x5 };
        p4 = { this_x - length_spl1x5 , this_y - width_spl1x5 };
        p5 = { this_x - length_spl1x5 - 0.2, this_y };
        p6 = { this_x , this_y + width_spl1x5 };
        p7 = { this_x - length_spl1x5 , this_y + width_spl1x5 };
        p8 = { this_x , this_y - width_spl1x5 / 2 };
        p9 = { this_x - length_spl1x5 - 0.1, this_y - width_spl1x5 / 2 };
        p10 = { this_x , this_y + width_spl1x5 / 2 };
        p11 = { this_x - length_spl1x5 - 0.1, this_y + width_spl1x5 / 2 };
        break;
    default:
        p2 = { -1 , -1 };
        p3 = { -1 , -1 };
        p4 = { -1 , -1 };
        p5 = { -1 , -1 };
        p6 = { -1 , -1 };
        p7 = { -1 , -1 };
        p8 = { -1 , -1 };
        p9 = { -1 , -1 };
        p10 = { -1 , -1 };
        p11 = { -1 , -1 };
        break;
    }

    // Based on adjustment point, make the routing
    // These waveguides do not cross and must not overlap 
    // The splitter is considered to have a specific area that must not overlap with other splitters or overlaps
    // The avoidance of this used area is not considered yet
    if (adj == point{ -width_spl1x5, length_spl1x5 } ||
        adj == point{ length_spl1x5, width_spl1x5 } ||
        adj == point{ width_spl1x5, -length_spl1x5 } ||
        adj == point{ -length_spl1x5, -width_spl1x5 }) {

        // Input 1 used
        waveguide* wg2 = new waveguide(p2, p3, 1, "Net : A ign");
        add_child(splitter_node, wg2);
        previous_node = splitter_node->children[subtree];

        bend* bd1 = new bend(p3, 0, "Bend : A ign");
        add_child(previous_node, bd1);
        previous_node = previous_node->children[0];

        waveguide* wg1 = new waveguide(p3, p4, 1, "Net : B ign");
        add_child(previous_node, wg1);
        return_node = previous_node->children[0];
        return_point = p4;
    }
    else if (adj == point{ 0, length_spl1x5 } ||
        adj == point{ length_spl1x5, 0 } ||
        adj == point{ 0, -length_spl1x5 } ||
        adj == point{ -length_spl1x5, 0 }) {

        // Input 3 used
        waveguide* wg3 = new waveguide(p2, p5, 1, "Net : C ign");
        add_child(splitter_node, wg3);
        return_node = splitter_node->children[subtree];
        return_point = p5;
    }
    else if (adj == point{ width_spl1x5 / 2, length_spl1x5 } ||
        adj == point{ length_spl1x5, -width_spl1x5 / 2 } ||
        adj == point{ -width_spl1x5 / 2, -length_spl1x5 } ||
        adj == point{ -length_spl1x5, width_spl1x5 / 2 }) {

        // Input 2 used
        waveguide* wg2 = new waveguide(p2, p10, 1, "Net : H ign");
        add_child(splitter_node, wg2);
        previous_node = splitter_node->children[subtree];

        bend* bd1 = new bend(p10, 0, "Bend : C ign");
        add_child(previous_node, bd1);
        previous_node = previous_node->children[0];

        waveguide* wg1 = new waveguide(p10, p11, 1, "Net : I ign");
        add_child(previous_node, wg1);
        return_node = previous_node->children[0];
        return_point = p11;
    }
    else if (adj == point{ -width_spl1x5 / 2, length_spl1x5 } ||
        adj == point{ length_spl1x5, width_spl1x5 / 2 } ||
        adj == point{ width_spl1x5 / 2, -length_spl1x5 } ||
        adj == point{ -length_spl1x5, -width_spl1x5 / 2 }) {

        // Input 4 used
        waveguide* wg2 = new waveguide(p2, p8, 1, "Net : J ign");
        add_child(splitter_node, wg2);
        previous_node = splitter_node->children[subtree];

        bend* bd1 = new bend(p8, 0, "Bend : D ign");
        add_child(previous_node, bd1);
        previous_node = previous_node->children[0];

        waveguide* wg1 = new waveguide(p8, p9, 1, "Net : K ign");
        add_child(previous_node, wg1);
        return_node = previous_node->children[0];
        return_point = p9;
    }
    else {

        // Input 5 used 
        waveguide* wg4 = new waveguide(p2, p6, 1, "Net : D ign");
        add_child(splitter_node, wg4);
        previous_node = splitter_node->children[subtree];

        bend* bd2 = new bend(p6, 0, "Bend : B ign");
        add_child(previous_node, bd2);
        previous_node = previous_node->children[0];

        waveguide* wg5 = new waveguide(p6, p7, 1, "Net : E ign");
        add_child(previous_node, wg5);
        return_node = previous_node->children[0];
        return_point = p7;
    } 

    return make_pair(return_node, return_point);
}


// Grouping Functions
// Utilized by the initGroupingMethod.h and COP-KMeans.h files
// -----------------------------------------------------------------------------------------------------------------------
tuple<vector<int>, vector<double>> sortedCentroids(vector<point>& centroids, point mp) {
    /**
    * Find the closest cluster for a specific point
    * Returns the centroids of all clusters sorted from closest to furthest
    * 
    * @param centroids The centroids
    * @param mp The merging point to find the closest cluster for
    */
    vector<double> distances;
    vector<int> indices(centroids.size());
    vector<point> sorted_centroids;

    for (const auto& centers : centroids) {
        distances.push_back(manhattan_distance(centers, mp));
    }

    iota(indices.begin(), indices.end(), 0);

    sort(indices.begin(), indices.end(),
        [&distances](int i1, int i2) { return distances[i1] < distances[i2]; });

    for (const auto& num : indices) {
        sorted_centroids.push_back(centroids[num]);
    }

    return make_tuple(indices, distances);
}

// -----------------------------------------------------------------------------------------------------------------------

bool constraintViolationChecker(vector<int>& centroidConstraint, int numSinks) {
    /**
    * Check if the number of sinks violates the constraints from the centroids used during COP-KMEANS grouping
    * true = The number of sinks does not match with the constraint of the specific centroid
    * false = The number of sinks matches one of the numbers in the constraint of the centroid, thus this MP can be grouped with the centroid
    * 
    * @param centroidConstraint The constraint of the centroid
    * @param numSinks The number of sinks corresponding to the specific centroid
    */
    bool violation = true;

    if (find(centroidConstraint.begin(), centroidConstraint.end(), numSinks) != centroidConstraint.end()) {
        violation = false;
        centroidConstraint.erase(find(centroidConstraint.begin(), centroidConstraint.end(), numSinks));
    }

    return violation;
}
// -----------------------------------------------------------------------------------------------------------------------

double euclideanDistance(const point& p1, const point& p2) {
    /**
    * Calculates the Euclidean distance between two points
    * 
    * @param p1 Point1
    * @param p2 Point2
    */
    return sqrt(pow(p1.x_coord - p2.x_coord, 2) + pow(p1.y_coord - p2.y_coord, 2));
}
// -----------------------------------------------------------------------------------------------------------------------

unordered_map<int, vector<point>> convertToMap(const vector<vector<point>>& groups) {
    /**
    * Converts a vector of vectors of points to a map which maps each vector of points with an integer corresponding to the group number
    * 
    * @param groups The vector of vectors
    */
    unordered_map<int, vector<point>> result;
    for (int i = 0; i < groups.size(); ++i) {
        result[i] = groups[i];
    }
    return result;
}

// -----------------------------------------------------------------------------------------------------------------------

unordered_map<point, vector<point>, pointHash> groupCreator(unordered_map<int, vector<point>> groups, unordered_map<int, point> MPs) {
    unordered_map<point, vector<point>, pointHash> grp_mps;
    for (int i = 0; i < groups.size(); i++) {
        grp_mps[MPs[i]] = groups[i];
    }
    return grp_mps;
}