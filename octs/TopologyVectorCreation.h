// This code is responsible for costructing the topology vector
// -----------------------------------------------------------------------------------------------------------------------

struct SynthSolution {
    /**
    * Structure containing the synthesis solution that creates the splitter from available ones
    * @param state Contains of integers corresponding to the number of sinks that have been grouped, starting with all ones
    * @param cost The cost defines the consumption by adding a heater in the solution
    * @param groupings Already existing groups of sinks
    * @param grouped_sinks Number of sinks that have already been grouped
    */
    vector<int> state;
    double cost;
    vector<vector<int>> groupings;
    int grouped_sinks;

    bool operator>(const SynthSolution& other) const {
        // Necessary for priority queue, orders based on the cost
        return cost > other.cost; 
    }

    vector<vector<vector<int>>> topology_subvector_creation(vector<vector<int>> groupings) {
        /**
        * Function that creates a subtopology vector based on the solution of synthesis
        * The subtopology vector corresponds to a prime whose splitter is not available
        * For example for 7 the subtopology vector will contain the solution of just how to group 7 sinks
        * So if we have 49 sinks = 7 * 7, the subtopology vector will still consist of only info of how to group 7 sinks
        * @param groupings The groupings solution found by the synthesis solver
        */
        vector<int> numbers = { 1 + grouped_sinks };
        vector<vector<vector<int>>> topology;

        while (!groupings.empty()) {
            vector<vector<int>> subgroup_depth = {};
            vector<int> new_numbers = { };
            vector<int> erase_vector = {};

            for (int i = 0; i < groupings.size(); i++) {
                int tot_sum = 0;
                int num_times_equal = 0;
                vector<int> shown_vector = {};

                for (int j = 0; j < groupings[i].size(); j++) {
                    for (int k = 0; k < numbers.size(); k++) {
                        if (groupings[i][j] == numbers[k]) {
                            num_times_equal += 1;
                            shown_vector.push_back(numbers[k]);
                        }
                    }
                    tot_sum += groupings[i][j];
                }
                if (num_times_equal == groupings[i].size()) {
                    subgroup_depth.push_back(groupings[i]);
                    erase_vector.push_back(i);
                    new_numbers.push_back(tot_sum);
                    shown_vector.clear();
                }
                else {
                    for (int n = 0; n < shown_vector.size(); n++) {
                        new_numbers.push_back(shown_vector[n]);
                        subgroup_depth.push_back({ shown_vector[n] });
                    }
                }
            }
            for (int m = 0; m < erase_vector.size(); m++) {
                groupings.erase(groupings.begin() + erase_vector[m] - m);
            }
            remove_duplicates(new_numbers);
            numbers = new_numbers;
            topology.push_back(subgroup_depth);
        }
        return topology;
    }
};

// -----------------------------------------------------------------------------------------------------------------------

bool contains_vec(const vector<vector<int>>& matrix, const vector<int>& targetVector) {
    /**
    * Function that checks if a specific vector is contained in another vector
    * @param matrix Vector of vectors of integers to search 
    * @param targetVector Vector of integers to be searched
    */
    return find(matrix.begin(), matrix.end(), targetVector) != matrix.end();
}

// -----------------------------------------------------------------------------------------------------------------------

bool contains_elem(const vector<int>& vec, int value) {
    /**
    * Function that checks if a vector contains a specific element (int)
    * @param vec The vector to search
    * @param value The integer value to be searched
    */
    return find(vec.begin(), vec.end(), value) != vec.end();
}

// -----------------------------------------------------------------------------------------------------------------------

void group_by_two(vector<int>& current_state, double& current_cost, vector<vector<int>>& current_groupings,
    priority_queue<SynthSolution, vector<SynthSolution>, greater<SynthSolution>>& next_pq) {
    /**
    * Function that groups sinks by two 
    * @param current_state Currently available sinks
    * @param current_cost Cost of the current state calculated based on the heater consumption
    * @param current_groupings Current groupings of sinks from current state
    * @param next_pq Currently available solutions saved in the priority queue
    */
    time_t start = clock();
    vector<vector<int>> all_states = {};

    for (size_t i = 0; i < current_state.size(); ++i) {
        for (size_t j = i + 1; j < current_state.size(); ++j) {
            vector<int> next_state;
            vector<int> grouped_elements;
            double cost = 0;

            for (size_t k = 0; k < current_state.size(); ++k) {
                if (k != i && k != j) {
                    next_state.push_back(current_state[k]);
                }
                else {
                    grouped_elements.push_back(current_state[k]);
                }
            }

            cost = estimate_group_power(grouped_elements);
            next_state.push_back(grouped_elements[0] + grouped_elements[1]);

            if (!contains_vec(all_states, next_state)) {
                SynthSolution next_sol_obj = { next_state, current_cost + cost, current_groupings };
                next_sol_obj.groupings.push_back(grouped_elements);
                next_pq.push(next_sol_obj);
                all_states.push_back(next_state);
            }
            else {
                continue;
            }
        }
    }
    time_t end = clock();
    if (timer) {
        setTextColor(CYAN);
        cout << "Time to group by 2 was " << (end - start) / double(CLOCKS_PER_SEC) << " seconds." << endl;
        setTextColor(WHITE);
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void group_by_three(vector<int>& current_state, double& current_cost, vector<vector<int>>& current_groupings,
    priority_queue<SynthSolution, vector<SynthSolution>, greater<SynthSolution>>& next_pq, map<int,int> counts) {
    /**
    * Function that groups sinks by three based on the availability of a 1x3 splitter
    * @param current_state Currently available sinks
    * @param current_cost Cost of the current state calculated based on the heater consumption
    * @param current_groupings Current groupings of sinks from current state
    * @param next_pq Currently available solutions saved in the priority queue
    * @param counts Map that contains how many subtrees contain X number of sinks
    */
    time_t start = clock();
    for (auto const& [val, count] : counts) {
        if (count >= 3) { 
            vector<int> next_state;
            vector<int> grouped_elements;
            int num_grouped = 0;

            for (int element : current_state) {
                if (element == val && num_grouped < 3) {
                    grouped_elements.push_back(element);
                    num_grouped++;
                }
                else {
                    next_state.push_back(element);
                }
            }

            double cost = estimate_group_power(grouped_elements);
            next_state.push_back(grouped_elements[0] + grouped_elements[1] + grouped_elements[2]);

            SynthSolution next_sol_obj = { next_state, current_cost + cost, current_groupings };
            next_sol_obj.groupings.push_back(grouped_elements);
            next_pq.push(next_sol_obj);
            break;
        }
    }
    time_t end = clock();
    if (timer) {
        setTextColor(CYAN);
        cout << "Time to group by 3 was " << (end - start) / double(CLOCKS_PER_SEC) << " seconds." << endl;
        setTextColor(WHITE);
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void group_by_five(vector<int>& current_state, double& current_cost, vector<vector<int>>& current_groupings,
    priority_queue<SynthSolution, vector<SynthSolution>, greater<SynthSolution>>& next_pq, map<int, int> counts) {
    /**
    * Function that groups sinks by five based on the availability of a 1x5 splitter
    * @param current_state Currently available sinks
    * @param current_cost Cost of the current state calculated based on the heater consumption
    * @param current_groupings Current groupings of sinks from current state
    * @param next_pq Currently available solutions saved in the priority queue
    * @param counts Map that contains how many subtrees contain X number of sinks
    */
    time_t start = clock();
    for (auto const& [val, count] : counts) {
        if (count >= 5) {  
            vector<int> next_state;
            vector<int> grouped_elements;
            int num_grouped = 0;

            for (int element : current_state) {
                if (element == val && num_grouped < 5) {
                    grouped_elements.push_back(element);
                    num_grouped++;
                }
                else {
                    next_state.push_back(element);
                }
            }

            double cost = estimate_group_power(grouped_elements);
            next_state.push_back(grouped_elements[0] + grouped_elements[1] + grouped_elements[2] + grouped_elements[3] + grouped_elements[4]);

            SynthSolution next_sol_obj = { next_state, current_cost + cost, current_groupings };
            next_sol_obj.groupings.push_back(grouped_elements);
            next_pq.push(next_sol_obj);
            break;
        }
    }
    time_t end = clock();
    if (timer) {
        setTextColor(CYAN);
        cout << "Time to group by 5 was " << (end - start) / double(CLOCKS_PER_SEC) << " seconds." << endl;
        setTextColor(WHITE);
    }
}

// -----------------------------------------------------------------------------------------------------------------------

SynthSolution synth(int num_sinks, int beam_width, vector<int> primes, int grouped_sinks) {
    /**
    * Function that automatically constructs the topology vector based on the available splitters
    * This function utilizes a priority queue the size of which is controled through the variable beam_width
    * The function utilizes Beam Search to account for large prime numbers that can increase the runtime significantly 
    * 
    * @param num_sinks Number of sinks in the clock tree
    * @param beam_width Can be controlled to change the size of the priority queue, thus the number of solutions that are examined each time 
    * Higher number leads to more precise solutions but increases runtime
    * @param primes Array containing the prime factors of the num_sinks number
    * @param grouped_sinks Number of grouped sinks
    */
    vector<int> initial_state(num_sinks, grouped_sinks + 1);

    priority_queue<SynthSolution, vector<SynthSolution>, greater<SynthSolution>> pq;
    priority_queue<SynthSolution, vector<SynthSolution>, greater<SynthSolution>> next_pq;
    priority_queue<SynthSolution, vector<SynthSolution>, greater<SynthSolution>> solved_pq;

    SynthSolution base_solution = { initial_state, 0, {} };
    pq.push(base_solution);

    int solutions_explored = 0;

    while (!pq.empty()) {
        // If the subtrees remaining are equal to 1 then add the synthesis solution to the solved_pq
        if (pq.top().state.size() == 1) {
            solved_pq.push(pq.top());
            solutions_explored++;
            pq.pop();
            next_pq.pop();
            // If the number of explored solutions exceeds the beam_width then return the best solution so far
            if (solutions_explored >= beam_width || pq.size() == 0) {
                return { {num_sinks}, solved_pq.top().cost, solved_pq.top().groupings };
            }
        }

        SynthSolution current_solution = pq.top();
        pq.pop();
        vector<int> current_state = current_solution.state;
        double current_cost = current_solution.cost;
        vector<vector<int>> current_groupings = current_solution.groupings;

        sort(current_state.begin(), current_state.end());

        // Counts the appearances of an integer
        map<int, int> counts;
        for (int val : current_state) {
            counts[val]++;
        }

        // Constructs the splitter based on available splitters
        try {
            if (contains_elem(splitter_avail, 2) && contains_elem(splitter_avail, 3) && contains_elem(splitter_avail, 5)) {
                group_by_five(current_state, current_cost, current_groupings, next_pq, counts);
                group_by_three(current_state, current_cost, current_groupings, next_pq, counts);
                group_by_two(current_state, current_cost, current_groupings, next_pq);
            }
            else if (contains_elem(splitter_avail, 2) && contains_elem(splitter_avail, 3)) {
                group_by_three(current_state, current_cost, current_groupings, next_pq, counts);
                group_by_two(current_state, current_cost, current_groupings, next_pq);
            }
            else if (contains_elem(splitter_avail, 2)) {
                group_by_two(current_state, current_cost, current_groupings, next_pq);
            }
            else {
                throw runtime_error("Splitter 1x2 is not found!");
            }
        } catch (const exception& e) {
            setTextColor(RED);
            cerr << "Error - Caught exception:" << e.what() << endl;
            exit(1);
        }

        pq = next_pq;
        priority_queue<SynthSolution, vector<SynthSolution>, greater<SynthSolution>> trimmed_pq;

        int count = 0;
        while (!pq.empty() && count < beam_width) {
            trimmed_pq.push(pq.top());
            pq.pop();
            count++;
        }
        pq = trimmed_pq;
        next_pq = trimmed_pq;
        if (next_pq.top().state.size() != 1) {
            next_pq.pop();
        }
    }

    SynthSolution empty_solution;
    return empty_solution;
}

// -----------------------------------------------------------------------------------------------------------------------

vector<vector<vector<int>>> topology_vector_creation(vector<int> primes, int sink_number, int trials = 20) {
    /**
    * Function that automatically constructs the topology vector based on the available splitters
    * The function groups the sinks together based on the prime numbers
    * If a splitter 1xY where Y is a prime is not available from the library then the algorithm constructs the splitter from already existing ones
    * trying to minimize the heater power consumption
    *
    * @param primes Vector consisting of the primes constructing the number of sinks
    * @param sink_number Number of sinks
    * @param trials Number of trials for finding the structure to construct a prime splitter
    * !Note that the algorithm sorts the prime array, meaning always focuses on adding large splitters first and at the end if adds the smaller ones
    */
    vector<vector<vector<int>>> top_vect = {};
    int remain = sink_number;
    int grouped_sinks = 1; // Initially the sinks are not grouped

    // Passing through each prime
    for (const auto& prime : primes) {
        // Calculates the remainder
        remain /= prime;
        // If the prime is not included in the available splitters
        if (!contains_elem(splitter_avail, prime)) {
            // Synthesizes a new splitter using available ones
            SynthSolution solution = synth(prime, trials, primes, grouped_sinks - 1);
            solution.grouped_sinks = grouped_sinks - 1;
            vector<vector<vector<int>>> short_top_vect = solution.topology_subvector_creation(solution.groupings);
            for (int m = 0; m < short_top_vect.size(); m++) {
                vector<vector<int>> extended_short_top_vect2 = {};
                for (int n = 0; n < short_top_vect[m].size(); n++) {
                    for (int r = 0; r < remain; r++) {
                        extended_short_top_vect2.push_back(short_top_vect[m][n]);
                    }
                }
                top_vect.push_back(extended_short_top_vect2);
            }
        }
        else {
            // If the prime is included in the available splitters
            vector<vector<int>> extended_short_top_vector;
            for (int i = 0; i < remain; i++) {
                vector<int> short_array;
                for (int k = 0; k < prime; k++) {
                    short_array.push_back(grouped_sinks);
                }
                extended_short_top_vector.push_back(short_array);
            }
            top_vect.push_back(extended_short_top_vector);
        }
        grouped_sinks *= prime;
    }
    return top_vect;
}

// -----------------------------------------------------------------------------------------------------------------------
// Older functions that can be used

string read_element_from_line(const string& filename, size_t line_number) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;

        for (size_t i = 0; i <= line_number; ++i) {
            if (!getline(file, line)) {
                return "";
            }
        }

        size_t space_pos = line.find('\n');
        string first_element = line.substr(0, space_pos);

        return first_element;
    }

    return "";
}

// Function to get the topology vector from a string
// The topology vector was produced through python code and was loaded in the CTS algorithm through this function
vector<vector<vector<int>>> get_topology_vector(const string& filename) {
    string whole_vector = read_element_from_line(filename, 0);
    vector<vector<vector<int>>> topology_vector;
    istringstream ss(whole_vector);

    char current_char = 'a';
    char prev_char = 'a';
    char prev_x2char = 'a';
    char prev_x3char = 'a';
    vector<vector<int>> sub_vector;
    vector<int> sub_sub_vector;
    while (ss >> current_char) {
        if ((current_char == '[' && prev_x2char == '[' && prev_char == '[') ||
            (current_char == '[' && prev_char == ',' && prev_x2char == ',') ||
            (current_char == '[' && prev_char == '[' && prev_x2char == ',') ||
            (current_char == '[' && prev_char == ',' && prev_x2char == '[')) {
            string inner_str;
            getline(ss, inner_str, ']');
            //cout << inner_str << endl;
            istringstream innerSS(inner_str);
            int num;
            while (innerSS >> num) {
                sub_sub_vector.push_back(num);
                innerSS >> current_char;
            }
            sub_vector.push_back(sub_sub_vector);
            sub_sub_vector.clear();
        }
        if ((prev_x2char == ',' && prev_char == ',' && current_char == ']') ||
            (prev_x2char == ']' && prev_char == ']') ||
            (prev_x2char == '[' && prev_char == ']')) {

            topology_vector.push_back(sub_vector);
            sub_vector.clear();
        }
        prev_x3char = prev_x2char;
        prev_x2char = prev_char;
        prev_char = current_char;
    }

    return topology_vector;
}
