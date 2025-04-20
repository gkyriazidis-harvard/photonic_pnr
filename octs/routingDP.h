// Routing Methodology
// -----------------------------------------------------------------------------------------------------------------------
point findCommonPoint(line line1, line line2) {
    /**
    * Function that finds the common point of two lines or returns {-1, -1} if it does not exist
    * 
    * @param line1 The first line
    * @param line2 The second line
    */
    double x1, y1, x2, y2, x3, y3, x4, y4;
    x1 = line1.p1.x_coord;
    y1 = line1.p1.y_coord;
    x2 = line1.p2.x_coord;
    y2 = line1.p2.y_coord;
    x3 = line2.p1.x_coord;
    y3 = line2.p1.y_coord;
    x4 = line2.p2.x_coord;
    y4 = line2.p2.y_coord;
    double intersection_x = -1;
    double intersection_y = -1;
    if (x1 == x2 && y3 == y4 && min(y1, y2) <= y3 && max(y1, y2) >= y3 && min(x3, x4) <= x1 && max(x3, x4) >= x1) {
        intersection_x = x1;
        intersection_y = y3;
    }
    else if (x3 == x4 && y1 == y2 && min(y3, y4) <= y1 && max(y3, y4) >= y1 && min(x1, x2) <= x3 && max(x1, x2) >= x3) {
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

bool overlapDetection(line line1, line line2) {
    /**
    * Function that checks if there is any kind of overlap between two lines
    * 
    * @param line1 The first line
    * @param line2 The second line
    */
    double x1, y1, x2, y2, x3, y3, x4, y4;
    x1 = line1.p1.x_coord;
    y1 = line1.p1.y_coord;
    x2 = line1.p2.x_coord;
    y2 = line1.p2.y_coord;
    x3 = line2.p1.x_coord;
    y3 = line2.p1.y_coord;
    x4 = line2.p2.x_coord;
    y4 = line2.p2.y_coord;

    if (x1 == x2 && x3 == x4 && x2 == x3) {
        if (not (y1 < y3 && y2 < y3 && y1 < y4 && y2 < y4) and
            not (y1 > y3 && y2 > y3 && y1 > y4 && y2 > y4)) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (y1 == y2 && y3 == y4 && y2 == y3) {
        if (not (x1 < x3 && x2 < x3 && x1 < x4 && x2 < x4) and
            not (x1 > x3 && x2 > x3 && x1 > x4 && x2 > x4)) {
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------------------

void connectNodes(Node* node1, Node* node2, Node* prev_node, double cost) {
    /**
    * Function that connects two nodes together
    * 
    * @param node1 The node to connect to
    * @param node2 The node to be connected
    * @param prev_node The previous node of node2
    * @param cost The power cost of connection between the nodes
    */
    pair<Node*, double> p1 = make_pair(prev_node, cost);
    node1->neighbors[node2].push_back(p1);
}

// -----------------------------------------------------------------------------------------------------------------------

bool containsPoint(const vector<Node*>& points, double x, double y) {
    /**
    * Function that checks if a vector of nodes contains a node with a specific point
    * 
    * @param points The vector of nodes to search in
    * @param x The x coordinate of the point being searched
    * @param y The y coordinate of the point being searched
    */
    for (const auto& pt : points) {
        if (pt->p == point{x , y}) {
            return true;
        }
    }
    return false;
}
// -----------------------------------------------------------------------------------------------------------------------

pair<double, vector<Node*>> dijkstra(Node* startNode, Node* endNode) {
    /**
    * Dijkstra algorithm altered to consider the previously visited nodes to account for the bend losses.
    * Returns the chosen nodes for the path as well as the total cost
    * 
    * @param startNode The Node from which Dijkstra begins.
    * @param endNode The target Node that Dijkstra is trying to reach. 
    */
    // Initialize distances to infinity for all nodes
    unordered_map<Node*, double> distances;
    unordered_map<Node*, vector<pair<Node*, double>>> neigh;
    vector<Node*> to_visit;
    to_visit.push_back(startNode);

    while (!to_visit.empty()) {
        neigh = to_visit[0]->neighbors;
        to_visit.erase(to_visit.begin());
        for (auto it = neigh.begin(); it != neigh.end(); it++) {
            //cout << to_visit.size() << endl;
            Node* neighbor = it->first;
            if (distances.find(neighbor) == distances.end()) {
                distances[neighbor] = numeric_limits<double>::infinity();
                to_visit.push_back(neighbor);
            }
        }
    }

    distances[startNode] = 0; // Distance to start node is 0

    // Priority queue to store nodes and their distances
    priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, NodeComparator> pq;
    pq.push({ 0, startNode });

    // Map to store the predecessor node for each node
    unordered_map<Node*, Node*> predecessors;
    predecessors[startNode] = nullptr;

    // Keep track of visited nodes using a vector
    vector<Node*> visited;

    while (!pq.empty()) {
        // Get the node with the smallest distance
        pair<double, Node*> tp = pq.top();
        double currentDistance = tp.first;
        Node* currentNode = tp.second;
        pq.pop();

        // If the node has already been visited, skip
        if (find(visited.begin(), visited.end(), currentNode) != visited.end()) {
            continue;
        }

        // Mark the node as visited
        visited.push_back(currentNode);

        // Iterate over neighbors of the current node
        for (const auto& part : currentNode->neighbors) {
            Node* neighborNode = part.first;
            vector<pair<Node*, double>> costs = part.second;
            // Find the cost to the neighbor based on the previous node
            double costToNeighbor = numeric_limits<double>::infinity();
            for (const auto& part2 : costs) {
                Node* prevNode = part2.first;
                double cost = part2.second;
                if (prevNode == predecessors[currentNode]) {
                    costToNeighbor = cost;
                    break;
                }
            }

            // If the new distance to the neighbor is shorter
            if (currentDistance + costToNeighbor < distances[neighborNode]) {
                // Update the distance to the neighbor
                distances[neighborNode] = currentDistance + costToNeighbor;
                // Update the predecessor of the neighbor
                predecessors[neighborNode] = currentNode;
                // Add the neighbor to the priority queue
                pq.push({ distances[neighborNode], neighborNode });
            }
        }
    }

    // Reconstruct the path from the end node to the start node
    vector<Node*> path;
    Node* currentNode = endNode;
    while (currentNode != nullptr) {
        path.push_back(currentNode);
        currentNode = predecessors[currentNode];
    }
    reverse(path.begin(), path.end());

    // Return the best distance found from path
    double shortestDistance = 0;
    Node* prev = nullptr;
    Node* next = path[1];
    for (int i = 0; i < path.size() - 1; i++) {
        vector<pair<Node*, double>> pair_old = path[i]->neighbors[next];
        double dist = 0;
        for (const auto& pair : pair_old) {
            // Check if the first item of the pair matches the known item
            if (pair.first == prev) {
                // If found, print the second item of the pair
                dist = pair.second;
            }
        }
        prev = path[i];
        if (i + 2 <= path.size() - 1) {
            next = path[i + 2];
        }
        shortestDistance += dist;
    }

    return make_pair(shortestDistance, path);
}

// -----------------------------------------------------------------------------------------------------------------------

Node* findNode(const vector<Node*>& nodes, double x, double y) {
    /**
    * Function that dins a specific node from a vector of nodes based on the given coordinates
    * @param nodes Vector of nodes to search in
    * @param x Coordinate in X axis being searched
    * @param y Coordinate in Y axis being searched
    */
    for (auto& node : nodes) {
        if (node->p == point{ x, y }) {
            return node;
        }
    }
    return nullptr;
}
// -----------------------------------------------------------------------------------------------------------------------

pair<vector<double>, vector<double>> nodePositionFinder(point p1, point p2, vector<line> horiz, vector<line> vert, double layers) {
    /**
    * A function that finds the x and y coordinates based on the starting/ending points the horizontal and vertical obstruction lines and the number of external layers
    * that we wish to add.
    * Returns a pair of two vectors that contain the x and y coordinates to be used to create the rectangular graph of Nodes.
    * @param p1 First point of the starting/ending pair.
    * @param p2 Second point of the starting/ending pair.
    * @param horiz Vector of horizontal lines from the obstructions.
    * @param vert Vector of vertical lines from the obstructions.
    * @param layers External layers that we want to add for routing to avoid overlaps.
    */
    vector<double> x_pos, y_pos;
    x_pos.push_back(p1.x_coord);
    x_pos.push_back(p2.x_coord);
    y_pos.push_back(p1.y_coord);
    y_pos.push_back(p2.y_coord);

    for (double l = 0; l < layers; l++) {
        x_pos.push_back(min(p1.x_coord, p2.x_coord) - 0.1 * (l + 1));
        x_pos.push_back(max(p1.x_coord, p2.x_coord) + 0.1 * (l + 1));
        y_pos.push_back(min(p1.y_coord, p2.y_coord) - 0.1 * (l + 1));
        y_pos.push_back(max(p1.y_coord, p2.y_coord) + 0.1 * (l + 1));
    }

    // Remove copies in vectors

    vector<double> x_uniq, y_uniq;
    for (int i = 0; i < x_pos.size(); ++i) {
        bool isDuplicate = false;
        for (int j = 0; j < i; ++j) {
            if (x_pos[i] == x_pos[j]) {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate) {
            x_uniq.push_back(x_pos[i]);
        }
    }

    for (int i = 0; i < y_pos.size(); ++i) {
        bool isDuplicate = false;
        for (int j = 0; j < i; ++j) {
            if (y_pos[i] == y_pos[j]) {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate) {
            y_uniq.push_back(y_pos[i]);
        }
    }

    x_pos = x_uniq;
    y_pos = y_uniq;

    if (p2.x_coord > p1.x_coord) {
        sort(x_pos.begin(), x_pos.end(), greater<double>());
    }
    else {
        sort(x_pos.begin(), x_pos.end());
    }

    if (p2.y_coord > p1.y_coord) {
        sort(y_pos.begin(), y_pos.end(), greater<double>());
    }
    else {
        sort(y_pos.begin(), y_pos.end());
    }

    // Add the mids of the numbers
    // This adds an extra freedom to the routing

    vector<double> x_positions;
    vector<double> y_positions;

    for (size_t i = 0; i < x_pos.size() - 1; i++) {
        double mid = (x_pos[i] + x_pos[i + 1]) / 2;
        x_positions.push_back(x_pos[i]);
        x_positions.push_back(mid);
    }
    x_positions.push_back(x_pos[x_pos.size() - 1]);

    for (size_t i = 0; i < y_pos.size() - 1; i++) {
        double mid = (y_pos[i] + y_pos[i + 1]) / 2;
        y_positions.push_back(y_pos[i]);
        y_positions.push_back(mid);
    }

    y_positions.push_back(y_pos[y_pos.size() - 1]);

    if (p2.x_coord > p1.x_coord) {
        sort(x_positions.begin(), x_positions.end(), greater<double>());
    }
    else {
        sort(x_positions.begin(), x_positions.end());
    }

    if (p2.y_coord > p1.y_coord) {
        sort(y_positions.begin(), y_positions.end(), greater<double>());
    }
    else {
        sort(y_positions.begin(), y_positions.end());
    }

    return make_pair(x_positions, y_positions);
}

// -----------------------------------------------------------------------------------------------------------------------

void connectionChecksEmpty(Node* n, Node* node, vector<line> all_lines, line l) {
    /**
    * Checks for crosses/overlaps and creates the connection of nodes with a suitable cost
    * 
    * @param n Node to connect with
    * @param node Node to be connected 
    * @param all_lines List of lines in the window used to check for crosses and overlaps
    * @param l Line created by connecting the two nodes
    */
    bool oc_detected = false;
    // Creates the cost based on the propagation loss
    double cost = manhattan_distance(n->p, node->p) * waveguide_prop_loss;

    for (const auto& ls : all_lines) {
        if (overlapDetection(ls, l)) {
            connectNodes(n, node, nullptr, numeric_limits<double>::max());
            oc_detected = true;
        }
        else if (findCommonPoint(ls, l) != point{ -1, -1 }) {
            connectNodes(n, node, nullptr, cost + cross_ins_loss);
            oc_detected = true;
        }
    }

    if (!oc_detected) {
        connectNodes(n, node, nullptr, cost);
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void connectionChecks(Node* n, Node* node, vector<line> all_lines, vector<double> x_positions, vector<double> y_positions, int direction, line l) {
    /**
    * 
    */
    double cost = manhattan_distance(node->p, n->p) * waveguide_prop_loss;
    for (const auto& conns : n->prev_conns) {
        bool oc_detected = false;
        //cout << "Connecting node " << n->p << " with " << node->p << " with previous node " << conns->p << endl;
        if (conns->p.x_coord == node->p.x_coord && conns->p.y_coord == node->p.y_coord) {
            connectNodes(n, node, conns, numeric_limits<double>::max());
        }
        else {
            for (const auto& ls : all_lines) {
                if (overlapDetection(ls, l)) {
                    //cout << "OVERLAP DETECTED! \n";
                    connectNodes(n, node, conns, numeric_limits<double>::max());
                    oc_detected = true;
                }
                else if (findCommonPoint(ls, l) != point{ -1, -1 }) {
                    if (conns->p.x_coord == node->p.x_coord || conns->p.y_coord == node->p.y_coord) {
                        connectNodes(n, node, conns, cost + cross_ins_loss);
                    }
                    else {
                        connectNodes(n, node, conns, cost + bend_ins_loss + cross_ins_loss);
                    }
                    oc_detected = true;
                }
            }

            if (node->p.x_coord == x_positions[x_positions.size() - 1] && node->p.y_coord == y_positions[y_positions.size() - 1] && !oc_detected) {
                if (direction == 0 && n->p.x_coord == node->p.x_coord) {
                    if (conns->p.x_coord == node->p.x_coord || conns->p.y_coord == node->p.y_coord) {
                        connectNodes(n, node, conns, cost);
                        //cout << "a1\n";
                    }
                    else {
                        connectNodes(n, node, conns, cost + bend_ins_loss);
                        //cout << "b1\n";
                    }
                }
                else if (direction == 1 && n->p.y_coord == node->p.y_coord) {
                    if (conns->p.x_coord == node->p.x_coord || conns->p.y_coord == node->p.y_coord) {
                        connectNodes(n, node, conns, cost);
                        //cout << "a1\n";
                    }
                    else {
                        connectNodes(n, node, conns, cost + bend_ins_loss);
                        //cout << "b1\n";
                    }
                }
                else {
                    connectNodes(n, node, conns, numeric_limits<double>::max());
                }
            }
            else if (!oc_detected) {
                if (conns->p.x_coord == node->p.x_coord || conns->p.y_coord == node->p.y_coord) {
                    connectNodes(n, node, conns, cost);
                }
                else {
                    connectNodes(n, node, conns, cost + bend_ins_loss);
                }
            }
        }
    }

}

// -----------------------------------------------------------------------------------------------------------------------

//! This code is currently not used by the OCTS algorithm
void exceptionThrowCheckPointInMultipleAreas(point p, vector<area> a) {
    for (const auto& ar : a) {
        if (p.x_coord >= ar.bl.x_coord && p.x_coord <= ar.br.x_coord) {
            if (p.y_coord >= ar.bl.y_coord && p.y_coord <= ar.ul.y_coord) {
                throw runtime_error("Point is inside the area!\n");
            }
        }
    }
}


// -----------------------------------------------------------------------------------------------------------------------

//! This code is currently not used by the OCTS algorithm
bool checkPointInMultipleAreas(point p, vector<area> a) {
    /**
    * Checks if a point is inside any of the areas in a vector
    * @param p Point to check
    * @param a Vector of area structures
    */
    for (const auto& ar : a) {
        if (p.x_coord >= ar.bl.x_coord && p.x_coord <= ar.br.x_coord) {
            if (p.y_coord >= ar.bl.y_coord && p.y_coord <= ar.ul.y_coord) {
                return true;
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------------------

bool containsPoint(const vector<Node*>& points, point test) {
    /**
    * Checks if there is a Node in a vector that corresponds to a specific Point
    * True if a Node with the corresponding Point is found.
    * @param points A vector of Nodes that needs to be searched.
    * @param test Point to be checked if it is inside the vector of Nodes.
    */
    for (const auto& pt : points) {
        if (pt->p == test) {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------------------

void connectNeighbors(Node* tbcd, Node* tc, vector<line> all_lines) {

    for (auto it = tbcd->neighbors.begin(); it != tbcd->neighbors.end(); it++) {
        if (it->first == tc) {
            connectNodes(tbcd, it->first, tc, numeric_limits<double>::max());
        }
        else {
            bool oc_detected = false;
            double cost = manhattan_distance(it->first->p, tbcd->p) * waveguide_prop_loss;
            line l = line{ it->first->p, tbcd->p };

            for (const auto& ls : all_lines) {
                if (overlapDetection(ls, l)) {
                    connectNodes(tbcd, it->first, tc, numeric_limits<double>::max());
                    oc_detected = true;
                }
                else if (findCommonPoint(ls, l) != point{ -1, -1 }) {
                    connectNodes(tbcd, it->first, tc, cost + cross_ins_loss);
                    oc_detected = true;
                }
            }

            if (!oc_detected) {
                connectNodes(tbcd, it->first, tc, cost);
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void extendGraph(unordered_map<point, Node*, NodeHash>& nodes_map, int layers, vector<double> x_positions, vector<double> y_positions, point& start, point& end,
    vector<line> all_lines, int direction, vector<area> violations) {

    vector<double> x_edge1, x_inter, x_edge2;
    vector<double> y_edge1, y_inter, y_edge2;

    // Filter to find the numbers
    size_t i = 0;
    for (; i < x_positions.size() && x_positions[i] != start.x_coord; ++i) {
        x_edge1.push_back(x_positions[i]);
    }

    reverse(x_edge1.begin(), x_edge1.end());
    size_t j = i;

    for (; j < x_positions.size() && x_positions[j] != end.x_coord; ++j) {
        x_inter.push_back(x_positions[j]);
    }

    x_inter.push_back(x_positions[j]);
    j++;
    size_t k = j;

    for (; k < x_positions.size(); ++k) {
        x_edge2.push_back(x_positions[k]);
    }

    i = 0;
    for (; i < y_positions.size() && y_positions[i] != start.y_coord; ++i) {
        y_edge1.push_back(y_positions[i]);
    }
    reverse(y_edge1.begin(), y_edge1.end());

    j = i;
    for (; j < y_positions.size() && y_positions[j] != end.y_coord; ++j) {
        y_inter.push_back(y_positions[j]);
    }

    y_inter.push_back(y_positions[j]);
    j++;
    k = j;

    for (; k < y_positions.size(); ++k) {
        y_edge2.push_back(y_positions[k]);
    }


    // Write the above as function at some point
    double prev_y = start.y_coord;
    for (const auto& ye : y_edge1) {
        for (const auto& xi : x_inter) {
            if (!checkPointInMultipleAreas(point{ xi, ye }, violations)) {
                Node* tc = nodes_map[point{ xi, prev_y }];
                Node* tbcd = nodes_map[point{ xi, ye }];
                line lin = line{ tc->p, tbcd->p };
                connectionChecks(tc, tbcd, all_lines, x_positions, y_positions, direction, lin);
                if (xi == start.x_coord && prev_y == start.y_coord) {
                    connectionChecksEmpty(tc, tbcd, all_lines, lin);
                }
                connectNeighbors(tbcd, tc, all_lines);
            }
        }
        prev_y = ye;
    }

    //---
    double prev_x = start.x_coord;
    for (const auto& xe : x_edge1) {
        for (const auto& yi : y_inter) {
            if (!checkPointInMultipleAreas(point{ xe, yi }, violations)) {
                Node* tc = nodes_map[point{ prev_x, yi }];
                Node* tbcd = nodes_map[point{ xe, yi }];
                line lin = line{ tc->p, tbcd->p };
                connectionChecks(tc, tbcd, all_lines, x_positions, y_positions, direction, lin);
                if (prev_x == start.x_coord && yi == start.y_coord) {
                    connectionChecksEmpty(tc, tbcd, all_lines, lin);
                }
                connectNeighbors(tbcd, tc, all_lines);
            }
        }
        prev_x = xe;
    }

    //---
    prev_y = y_inter[y_inter.size() - 1];
    for (const auto& ye : y_edge2) {
        for (const auto& xi : x_inter) {
            if (!checkPointInMultipleAreas(point{ xi, ye }, violations)) {
                Node* tbcd = nodes_map[point{ xi, prev_y }];
                Node* tc = nodes_map[point{ xi, ye }];
                line lin = line{ tc->p, tbcd->p };
                connectionChecks(tc, tbcd, all_lines, x_positions, y_positions, direction, lin);
                connectNeighbors(tbcd, tc, all_lines);
            }
        }
        prev_y = ye;
    }

    //---
    prev_x = x_inter[x_inter.size() - 1];
    for (const auto& xe : x_edge2) {
        for (const auto& yi : y_inter) {
            if (!checkPointInMultipleAreas(point{ xe, yi }, violations)) {
                Node* tbcd = nodes_map[point{ prev_x, yi }];
                Node* tc = nodes_map[point{ xe, yi }];
                line lin = line{ tc->p, tbcd->p };
                connectionChecks(tc, tbcd, all_lines, x_positions, y_positions, direction, lin);
                connectNeighbors(tbcd, tc, all_lines);
            }
        }
        prev_x = xe;
    }
}

// -----------------------------------------------------------------------------------------------------------------------
// direction 0 -> horizontal, 1 -> vertical
unordered_map<point, Node*, NodeHash> graphCreation(point p1, point p2, int direction, pair<vector<double>, vector<double>> xy, vector<line> all_lines,
    int ext_layers, point real_start, point real_finish, vector<area> violations) {

    vector<double> x_positions = xy.first;
    vector<double> y_positions = xy.second;

    // x,y setup complete
    int i = 0;
    int j = 0;

    vector<Node*> prev_layer;
    vector<Node*> curr_layer;
    unordered_map<point, Node*, NodeHash> nodes_map;

    Node* start = new Node{ point{x_positions[0], y_positions[0]}, i, j };
    prev_layer.push_back(start);
    nodes_map[point{ x_positions[0], y_positions[0] }] = start;

    // Code used to check if the starting or ending points are inside an obstructed area
    /*
    try {
        exceptionThrowCheckPointInMultipleAreas(point{ x_positions[0], y_positions[0] }, violations);
    }
    catch (const exception& e) {
        setTextColor(RED);
        cerr << "Starting point is overlapping with an obstructing area!\n";
        exit(0);
    }

    try {
        exceptionThrowCheckPointInMultipleAreas(point{ x_positions[x_positions.size() - 1], y_positions[y_positions.size() - 1] }, violations);
    }
    catch (const exception& e) {
        setTextColor(RED);
        cerr << "Ending point is overlapping with an obstructing area!\n";
        exit(0);
    }*/

    point test = point{ x_positions[x_positions.size() - 1], y_positions[y_positions.size() - 1] };

    while (!containsPoint(prev_layer, test)) {
        //cout << prev_layer.size() << endl;
        for (const auto& n : prev_layer) {
            i = n->i;
            j = n->j;
            if (i + 1 < x_positions.size() && !checkPointInMultipleAreas(point{ x_positions[i + 1], y_positions[j] }, violations)) {
                Node* node;
                test = point{ x_positions[i + 1], y_positions[j] };
                if (!containsPoint(curr_layer, test)) {
                    node = new Node{ x_positions[i + 1], y_positions[j], i + 1, j };
                    //cout << "LOADED1 = " << x_positions[i + 1] << " & " << y_positions[j] << endl;
                    curr_layer.push_back(node);
                    nodes_map[point{ x_positions[i + 1], y_positions[j] }] = node;
                }
                else {
                    //cout << "Found\n";
                    node = findNode(curr_layer, x_positions[i + 1], y_positions[j]);
                }
                //cout << "Size" << n->prev_conns.size() << endl;
                line l = { n->p, node->p };

                connectionChecks(n, node, all_lines, x_positions, y_positions, direction, l);
                if (n->prev_conns.empty()) {
                    connectionChecksEmpty(n, node, all_lines, l);
                }

                node->prev_conns.push_back(n);
            }
            if (j + 1 < y_positions.size() && !checkPointInMultipleAreas(point{ x_positions[i], y_positions[j + 1] }, violations)) {
                Node* node;
                test = { x_positions[i], y_positions[j + 1] };
                if (!containsPoint(curr_layer, test)) {
                    node = new Node{ x_positions[i], y_positions[j + 1], i, j + 1 };
                    //cout << "LOADED2 = " << x_positions[i] << " & " << y_positions[j + 1] << endl;
                    curr_layer.push_back(node);
                    nodes_map[point{ x_positions[i], y_positions[j + 1] }] = node;
                }
                else {
                    node = findNode(curr_layer, x_positions[i], y_positions[j + 1]);
                }

                line l = { n->p, node->p };

                connectionChecks(n, node, all_lines, x_positions, y_positions, direction, l);
                if (n->prev_conns.empty()) {
                    connectionChecksEmpty(n, node, all_lines, l);
                }

                node->prev_conns.push_back(n);
            }
        }
        prev_layer = curr_layer;
        curr_layer.clear();
        test = point{ x_positions[x_positions.size() - 1], y_positions[y_positions.size() - 1] };
        //cout << "Repeat" << endl;
    }


    Node* ns = nodes_map[real_start];
    if (ext_layers != 0) {
        for (auto it = ns->neighbors.begin(); it != ns->neighbors.end(); it++) {
            line lin = line{ it->first->p, ns->p };
            connectionChecksEmpty(ns, it->first, all_lines, lin);
        }
    }

    Node* finalNode = prev_layer[0];
    point finish = finalNode->p;

    // Create the extra layers if there are any needed
    point begin = start->p;
    extendGraph(nodes_map, ext_layers, x_positions, y_positions, real_finish, real_start, all_lines, direction, violations);

    //cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    // Print connections and their costs
    for (auto it = nodes_map.begin(); it != nodes_map.end(); ++it) {
        point pos = it->first;
        Node* node = it->second;
        //cout << "Node " << node->p << " is connected to: ";
        for (auto neighbor_it = node->neighbors.begin(); neighbor_it != node->neighbors.end(); ++neighbor_it) {
            Node* neighbor = neighbor_it->first;
            vector<pair<Node*, double>> small_map = neighbor_it->second;
            for (const auto& nodey : small_map) {
                if (nodey.first != nullptr) {
                    //cout << neighbor->p << " coming from " << nodey.first->p << " with cost " << nodey.second << " \n";
                }
                else {
                    //cout << neighbor->p << " coming from nowhere with cost " << nodey.second << " \n";
                }
            }
        }
        //cout << endl;
    }
    //cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";

    return nodes_map;
}

// -----------------------------------------------------------------------------------------------------------------------

tuple<vector<line>, vector<point>> traverseGraph(vector<nary_tree_node*> tops, point real_start, point real_end, int direction) {
    /**
    * Function that traverses the graph created for routing
    * 
    * @param tops The vector containing the top nodes of the graph
    * @param real_start The starting point of the routing
    * @param real_end The ending point of the routing
    * @param direction Integer value (should be changed to boolean) signifying the initial direction of the route from the starting point (horizontal or vertical)
    */
    // p1 is splitter top MP
    pair<vector<line>, vector<line>> window = make_pair(vector<line>(), vector<line>());
    int layers = 0;
    double cost = numeric_limits<double>::max();
    point ps = real_start;
    point pe = real_end;
    vector<area> viols;
    vector<line> all_lines;
    vector<Node*> path;

    // layers can be limited here
    int max_num_layers = 2;
    while (cost == numeric_limits<double>::max() && layers < max_num_layers) {

        // For each subtree check for overlaps
        for (int i = 0; i < tops.size(); i++) {
            window = DFSWindowSearcher(tops[i], window, ps, pe);
        }

        // By default we start with horizontal routing first
        // Starting point is p2
        vector<line> horiz = window.first;
        vector<line> vert = window.second;
        all_lines.resize(horiz.size() + vert.size());
        copy(horiz.begin(), horiz.end(), all_lines.begin());
        copy(vert.begin(), vert.end(), back_inserter(all_lines));

        pair<vector<double>, vector<double>> xy_pos = nodePositionFinder(real_start, real_end, horiz, vert, layers);

        unordered_map<point, Node*, NodeHash> nodes_map = graphCreation(ps, pe, direction, xy_pos, all_lines, layers, real_start, real_end, viols);

        pair<double, vector<Node*>> route_result = dijkstra(nodes_map[real_end], nodes_map[real_start]);

        cost = route_result.first;
        path = route_result.second;

        layers++;

        if (min(ps.x_coord, pe.x_coord) == ps.x_coord) {
            if (min(ps.y_coord, pe.y_coord) == ps.y_coord) {
                ps = point{ ps.x_coord - 0.1, ps.y_coord - 0.1 };
                pe = point{ pe.x_coord + 0.1, pe.y_coord + 0.1 };
            }
            else {
                ps = point{ ps.x_coord - 0.1, ps.y_coord + 0.1 };
                pe = point{ pe.x_coord + 0.1, pe.y_coord - 0.1 };
            }
        }
        else {
            if (min(ps.y_coord, pe.y_coord) == ps.y_coord) {
                ps = point{ ps.x_coord + 0.1, ps.y_coord - 0.1 };
                pe = point{ pe.x_coord - 0.1, pe.y_coord + 0.1 };
            }
            else {
                ps = point{ ps.x_coord + 0.1, ps.y_coord + 0.1 };
                pe = point{ pe.x_coord - 0.1, pe.y_coord - 0.1 };
            }
        }
    }

    /*
    cout << "-------OUTPUTS-----\n";
    cout << "Total Cost is : " << cost << endl;
    cout << "Layers used : " << layers - 1 << endl;
    for (const auto& nn : path) {
        cout << "Path point " << nn->p << endl;
    }
    */

    vector<point> crosses;
    Node* first = path[0];
    vector<line> prewg;

    Node* start = path[0];
    string status = "Nothing";
    string prev_status = "Nothing";
    Node* currentNode = nullptr;

    for (int i = 1; i < path.size(); i++) {
        currentNode = path[i];
        if (currentNode->p.x_coord == start->p.x_coord) {
            status = "Vertical";
        }
        else {
            status = "Horizontal";
        }
        //cout << status << endl;

        if (prev_status != status && i != 1) {
            //cout << "In\n";
            line l1 = { first->p, start->p };
            prewg.push_back(l1);
            first = start;
            for (const auto& ls : all_lines) {
                if (findCommonPoint(ls, l1) != point{ -1, -1 }) {
                    //cout << "CROSS FOUND!\n";
                    crosses.push_back(findCommonPoint(ls, l1));
                    //cout << findCommonPoint(ls, l1) << endl;
                }
            }
        }

        prev_status = status;
        start = path[i];
    }

    line l1 = { first->p, path[path.size() - 1]->p };
    for (const auto& ls : all_lines) {
        if (findCommonPoint(ls, l1) != point{ -1, -1 }) {
            //cout << "CROSS FOUND!\n";
            crosses.push_back(findCommonPoint(ls, l1));
            //cout << findCommonPoint(ls, l1) << endl;
        }
    }

    prewg.push_back(l1);



    return make_tuple(prewg, crosses);
}
// -----------------------------------------------------------------------------------------------------------------------