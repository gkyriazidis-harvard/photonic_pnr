namespace operations_research {
    tuple<vector<point>, unordered_map<point, double, pointHash>> optimizer_mrs(unordered_map<int, vector<point>> groups,
        unordered_map<point, vector<int>, pointHash> point_dev_map,
        unordered_map<point, vector<double>, pointHash> power_min_max,
        unordered_map<point, vector<double>, pointHash> dist_min_max,
        int number_of_variables) {

        vector<point> solutions;
        vector<double> SRs;
        unordered_map<point, double, pointHash> mapSRs;

        // [START solver creation]
        unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
        if (!solver) {
            setTextColor(RED);
            cerr << "SCIP solver unavailable!";
            exit(EXIT_FAILURE);
        }
        // [END solver creation]

        // [START variables]
        const double infinity = solver->infinity();
        vector<MPVariable*> variables;
        for (int i = 0; i < number_of_variables; i++) {
            MPVariable* const x = solver->MakeNumVar(0, dim_x, "x" + to_string(i));
            variables.push_back(x);
            MPVariable* const y = solver->MakeNumVar(0, dim_y, "y" + to_string(i));
            variables.push_back(y);
        }
        // [END variables]

        // [START constraints]
        vector<MPConstraint*> all_constraints;

        // [Constraint 1: BOX]
        if (debugger) {
            setTextColor(YELLOW);
            cout << "Commences box (bend) constraint creation..." << endl;
            setTextColor(WHITE);
        }
        for (int label = 0; label < number_of_variables; label++) {

            // Find the respective points for each group
            vector<point> this_group;
            auto it = groups.find(label);
            if (it != groups.end()) {
                this_group = it->second;
            }

            // Get the minimum and maximum values for the x-axis
            double min_value_x = numeric_limits<double>::max();
            double max_value_x = numeric_limits<double>::min();

            for (const auto& point : this_group) {
                if (point.x_coord < min_value_x) {
                    min_value_x = point.x_coord;
                }
            }
            for (const auto& point : this_group) {
                if (point.x_coord > max_value_x) {
                    max_value_x = point.x_coord;
                }
            }

            // Get the minimum and maximum values for the y-axis
            double min_value_y = numeric_limits<double>::max();
            double max_value_y = numeric_limits<double>::min();

            for (const auto& point : this_group) {
                if (point.y_coord < min_value_y) {
                    min_value_y = point.y_coord;
                }
            }
            for (const auto& point : this_group) {
                if (point.y_coord > max_value_y) {
                    max_value_y = point.y_coord;
                }
            }

            // We need bends from both sides, however the points can be in the same line!
            //! Bend radius must not be too large
            if (max_value_x - min_value_x < bend_radius * 2) {
                MPConstraint* const below_x = solver->MakeRowConstraint(min_value_x, +infinity);
                below_x->SetCoefficient(variables[2 * label], 1);
                MPConstraint* const upper_x = solver->MakeRowConstraint(-infinity, max_value_x);
                upper_x->SetCoefficient(variables[2 * label], 1);
            }
            else {
                MPConstraint* const below_x = solver->MakeRowConstraint(min_value_x + bend_radius, +infinity);
                below_x->SetCoefficient(variables[2 * label], 1);
                MPConstraint* const upper_x = solver->MakeRowConstraint(-infinity, max_value_x - bend_radius);
                upper_x->SetCoefficient(variables[2 * label], 1);
            }

            // Calculate constraints for y-axis
            if (max_value_y - min_value_y < 0.1) {
                MPConstraint* const below_y = solver->MakeRowConstraint(min_value_y, +infinity);
                below_y->SetCoefficient(variables[2 * label + 1], 1);
                MPConstraint* const upper_y = solver->MakeRowConstraint(-infinity, max_value_y);
                upper_y->SetCoefficient(variables[2 * label + 1], 1);
            }
            else {
                MPConstraint* const below_y = solver->MakeRowConstraint(min_value_y + 0.05, +infinity);
                below_y->SetCoefficient(variables[2 * label + 1], 1);
                MPConstraint* const upper_y = solver->MakeRowConstraint(-infinity, max_value_y - 0.05);
                upper_y->SetCoefficient(variables[2 * label + 1], 1);
            }
        }
        if (debugger) {
            setTextColor(LIGHTGREEN);
            cout << "Box (bend) constraints created successfully!" << endl;
            setTextColor(WHITE);
        }
        // [End Constraint 1: BOX]

        // [Constraint 2: TIME]
        if (debugger) {
            setTextColor(YELLOW);
            cout << "Commences delay constraint creation..." << endl;
            setTextColor(WHITE);
        }
        vector<int> indexes;
        for (int label = 0; label < number_of_variables; label++) {
            // Find the respective points for each group
            vector<point> this_group;
            auto it = groups.find(label);
            if (it != groups.end()) {
                this_group = it->second;
            }

            size_t index = 0;

            // Combinations between the group points
            for (int i = 0; i < this_group.size(); i++) {
                for (int j = i + 1; j < this_group.size(); j++) {

                    point p1 = { this_group[i].x_coord, this_group[i].y_coord };
                    point p2 = { this_group[j].x_coord, this_group[j].y_coord };

                    // Retrieve the vector of delays corresponding to the search key
                    vector<double>& values1 = dist_min_max[p1];
                    vector<double>& values2 = dist_min_max[p2];

                    double lmin_a = values1[0];
                    double lmax_a = values1[1];
                    double lmin_b = values2[0];
                    double lmax_b = values2[1];

                    /*cout << "lmina = " << lmin_a << endl;
                    cout << "lmaxa = " << lmax_a << endl;
                    cout << "lminb = " << lmin_b << endl;
                    cout << "lmaxb = " << lmax_b << endl;*/

                    // Calculate the other term
                    double other_term = c * thres / ng;

                    // Find the length differences for each subtree
                    double length_difference1 = (lmax_a - lmin_b) * c / ng;
                    double length_difference2 = (lmin_a - lmax_b) * c / ng;
                    //cout << "ldiff1 = " << length_difference1 << endl;
                    //cout << "ldiff2 = " << length_difference2 << endl;

                    // Find the mean of the length differences, this is the internal term
                    double mean_difference = (length_difference2 + length_difference1) / 2;
                    //cout << "mean_diff = " << mean_difference << endl;

                    // Find the external max skew 
                    double the_max_difference = max(length_difference1, length_difference2);
                    //cout << "max_diff = " << the_max_difference << endl;

                    // Find the adjustment term
                    double adjustment_term = the_max_difference - mean_difference;
                    //cout << "adj_term = " << adjustment_term << endl;
                    //cout << "CalculationsFinished" << endl;

                    // Define auxiliary variables for the absolute values
                    MPVariable* abs_x_1 = solver->MakeNumVar(0.0, +infinity, "abs_x1");
                    MPVariable* abs_y_2 = solver->MakeNumVar(0.0, +infinity, "abs_y2");
                    MPVariable* abs_x_3 = solver->MakeNumVar(0.0, +infinity, "abs_x3");
                    MPVariable* abs_y_4 = solver->MakeNumVar(0.0, +infinity, "abs_y4");

                    MPVariable* binary1 = solver->MakeBoolVar("bool1");
                    MPVariable* binary2 = solver->MakeBoolVar("bool2");
                    MPVariable* binary3 = solver->MakeBoolVar("bool3");
                    MPVariable* binary4 = solver->MakeBoolVar("bool4");

                    index = variables.size();
                    variables.push_back(abs_x_1);
                    variables.push_back(abs_y_2);
                    variables.push_back(abs_x_3);
                    variables.push_back(abs_y_4);
                    variables.push_back(binary1);
                    variables.push_back(binary2);
                    variables.push_back(binary3);
                    variables.push_back(binary4);
                    //cout << "VariablesPushed" << endl;

                    // Absolute extra constraints
                    // First Absolute value with xa
                    double value1, value2, value3, value4;

                    value1 = this_group[i].x_coord;
                    value2 = this_group[i].y_coord;
                    value3 = this_group[j].x_coord;
                    value4 = this_group[j].y_coord;

                    // First Absolute value with Xa
                    MPConstraint* const abs1_x_1 = solver->MakeRowConstraint(value1, +infinity);
                    abs1_x_1->SetCoefficient(variables[2 * label], 1);
                    abs1_x_1->SetCoefficient(variables[index], 1);
                    MPConstraint* const abs2_x_1 = solver->MakeRowConstraint(-value1, +infinity);
                    abs2_x_1->SetCoefficient(variables[2 * label], -1);
                    abs2_x_1->SetCoefficient(variables[index], 1);
                    // Big M Constraints
                    MPConstraint* const abs1_x_M1 = solver->MakeRowConstraint(-infinity, value1);
                    abs1_x_M1->SetCoefficient(variables[2 * label], 1);
                    abs1_x_M1->SetCoefficient(variables[index], 1);
                    abs1_x_M1->SetCoefficient(variables[index + 4], -M);
                    MPConstraint* const abs2_x_M1 = solver->MakeRowConstraint(-infinity, -value1 + M);
                    abs2_x_M1->SetCoefficient(variables[2 * label], -1);
                    abs2_x_M1->SetCoefficient(variables[index], 1);
                    abs2_x_M1->SetCoefficient(variables[index + 4], M);

                    // Second Absolute value with Ya
                    MPConstraint* const abs1_y_2 = solver->MakeRowConstraint(value2, +infinity);
                    abs1_y_2->SetCoefficient(variables[2 * label + 1], 1);
                    abs1_y_2->SetCoefficient(variables[index + 1], 1);
                    MPConstraint* const abs2_y_2 = solver->MakeRowConstraint(-value2, +infinity);
                    abs2_y_2->SetCoefficient(variables[2 * label + 1], -1);
                    abs2_y_2->SetCoefficient(variables[index + 1], 1);
                    // Big M Constraints
                    MPConstraint* const abs1_y_M2 = solver->MakeRowConstraint(-infinity, value2);
                    abs1_y_M2->SetCoefficient(variables[2 * label + 1], 1);
                    abs1_y_M2->SetCoefficient(variables[index + 1], 1);
                    abs1_y_M2->SetCoefficient(variables[index + 5], -M);
                    MPConstraint* const abs2_y_M2 = solver->MakeRowConstraint(-infinity, -value2 + M);
                    abs2_y_M2->SetCoefficient(variables[2 * label + 1], -1);
                    abs2_y_M2->SetCoefficient(variables[index + 1], 1);
                    abs2_y_M2->SetCoefficient(variables[index + 5], M);

                    //Third Absolute value with Xb
                    MPConstraint* const abs1_x_3 = solver->MakeRowConstraint(value3, +infinity);
                    abs1_x_3->SetCoefficient(variables[2 * label], 1);
                    abs1_x_3->SetCoefficient(variables[index + 2], 1);
                    MPConstraint* const abs2_x_3 = solver->MakeRowConstraint(-value3, +infinity);
                    abs2_x_3->SetCoefficient(variables[2 * label], -1);
                    abs2_x_3->SetCoefficient(variables[index + 2], 1);
                    // Big M Constraints
                    MPConstraint* const abs1_x_M3 = solver->MakeRowConstraint(-infinity, value3);
                    abs1_x_M3->SetCoefficient(variables[2 * label], 1);
                    abs1_x_M3->SetCoefficient(variables[index + 2], 1);
                    abs1_x_M3->SetCoefficient(variables[index + 6], -M);
                    MPConstraint* const abs2_x_M3 = solver->MakeRowConstraint(-infinity, -value3 + M);
                    abs2_x_M3->SetCoefficient(variables[2 * label], -1);
                    abs2_x_M3->SetCoefficient(variables[index + 2], 1);
                    abs2_x_M3->SetCoefficient(variables[index + 6], M);

                    // Fourth Absolute value with Yb
                    MPConstraint* const abs1_y_4 = solver->MakeRowConstraint(value4, +infinity);
                    abs1_y_4->SetCoefficient(variables[2 * label + 1], 1);
                    abs1_y_4->SetCoefficient(variables[index + 3], 1);
                    MPConstraint* const abs2_y_4 = solver->MakeRowConstraint(-value4, +infinity);
                    abs2_y_4->SetCoefficient(variables[2 * label + 1], -1);
                    abs2_y_4->SetCoefficient(variables[index + 3], 1);
                    // Big M Constraints
                    MPConstraint* const abs1_y_M4 = solver->MakeRowConstraint(-infinity, value4);
                    abs1_y_M4->SetCoefficient(variables[2 * label + 1], 1);
                    abs1_y_M4->SetCoefficient(variables[index + 3], 1);
                    abs1_y_M4->SetCoefficient(variables[index + 7], -M);
                    MPConstraint* const abs2_y_M4 = solver->MakeRowConstraint(-infinity, -value4 + M);
                    abs2_y_M4->SetCoefficient(variables[2 * label + 1], -1);
                    abs2_y_M4->SetCoefficient(variables[index + 3], 1);
                    abs2_y_M4->SetCoefficient(variables[index + 7], M);

                    MPConstraint* const time_cons1 = solver->MakeRowConstraint(-infinity, other_term
                        - adjustment_term - mean_difference);
                    time_cons1->SetCoefficient(variables[index], 1);
                    time_cons1->SetCoefficient(variables[index + 1], 1);
                    time_cons1->SetCoefficient(variables[index + 2], -1);
                    time_cons1->SetCoefficient(variables[index + 3], -1);

                    MPConstraint* const time_cons2 = solver->MakeRowConstraint(-other_term
                        + adjustment_term - mean_difference, +infinity);
                    time_cons2->SetCoefficient(variables[index], 1);
                    time_cons2->SetCoefficient(variables[index + 1], 1);
                    time_cons2->SetCoefficient(variables[index + 2], -1);
                    time_cons2->SetCoefficient(variables[index + 3], -1);

                }
            }
            indexes.push_back(static_cast<int>(index));
        } // [End Constraint 2: TIME]
        if (debugger) {
            setTextColor(LIGHTGREEN);
            cout << "Delay constraints created successfully!" << endl;
            setTextColor(WHITE);
        }

        if (debugger) {
            setTextColor(YELLOW);
            cout << "Commences power constraint creation..." << endl;
            setTextColor(WHITE);
        }
        // [Constraint 3: POWER]
        for (int label = 0; label < number_of_variables; label++) {

            // Find the respective points for each group
            vector<point> this_group;
            auto it = groups.find(label);
            if (it != groups.end()) {
                this_group = it->second;
            }

            // Combinations between the group points
            for (int i = 0; i < this_group.size(); i++) {
                for (int j = i + 1; j < this_group.size(); j++) {

                    point p1 = { this_group[i].x_coord, this_group[i].y_coord };
                    point p2 = { this_group[j].x_coord, this_group[j].y_coord };

                    // Retrieve the vector of integers corresponding to the search key
                    vector<double>& values1 = power_min_max[p1];
                    vector<double>& values2 = power_min_max[p2];

                    double pmin_a = values1[0];
                    double pmax_a = values1[1];
                    double pmin_b = values2[0];
                    double pmax_b = values2[1];

                    // Calculate the other term
                    double other_side = power_thres / waveguide_prop_loss;

                    // Find the length differences for each subtree
                    double power_difference1 = pmax_a - pmin_b;
                    double power_difference2 = pmin_a - pmax_b;

                    // Find the mean of the length differences, this is the internal term
                    double mean_difference = (power_difference2 + power_difference1) / 2;

                    // Find the external max skew 
                    double the_max_difference = max(power_difference1, power_difference2);

                    // Find the adjustment term
                    double adjustment_term = the_max_difference - mean_difference;

                    int sinks1 = point_dev_map[p1][3];
                    int sinks2 = point_dev_map[p2][3];
                    int bends1 = point_dev_map[p1][0];
                    int bends2 = point_dev_map[p2][0];
                    int cross1 = point_dev_map[p1][1];
                    int cross2 = point_dev_map[p2][1];
                    int splitter1 = point_dev_map[p1][2];
                    int splitter2 = point_dev_map[p2][2];
                    double SR = 0.0;

                    // Control the SR
                    if (sinks1 != sinks2 && this_group.size() == 2) {
                        double a = ( pmax_a + (- pmax_a + pmin_a) / 2) - (pmax_b + (- pmax_b + pmin_b) / 2);
                        double a_pow = pow(10, a/10);
                        SR = 1 / (1 + a_pow);
                    }
                    else {
                        SR = 0.5;
                    }
                    SRs.push_back(SR);

                    int index = indexes[label];

                    double abs_value_term = log10((1 - SR) / SR) - 1 * splitter_ins_loss_1x2 / waveguide_prop_loss;

                    MPConstraint* const power_cons1 = solver->MakeRowConstraint(-infinity, (other_side
                        - adjustment_term - mean_difference) / waveguide_prop_loss - abs_value_term);
                    power_cons1->SetCoefficient(variables[index], 1);
                    power_cons1->SetCoefficient(variables[index + 1], 1);
                    power_cons1->SetCoefficient(variables[index + 2], -1);
                    power_cons1->SetCoefficient(variables[index + 3], -1);

                    MPConstraint* const power_cons2 = solver->MakeRowConstraint((-other_side
                        + adjustment_term - mean_difference) / waveguide_prop_loss - abs_value_term, +infinity);
                    power_cons2->SetCoefficient(variables[index], 1);
                    power_cons2->SetCoefficient(variables[index + 1], 1);
                    power_cons2->SetCoefficient(variables[index + 2], -1);
                    power_cons2->SetCoefficient(variables[index + 3], -1);

                }
            }
        } // [End Constraint 3: POWER]
        if (debugger) {
            setTextColor(LIGHTGREEN);
            cout << "Power constraints created successfully!" << endl;
            setTextColor(WHITE);
        }

        if (debugger) {
            setTextColor(YELLOW);
            cout << "Commences distance constraint creation..." << endl;
            setTextColor(WHITE);
        }
        vector<int> other_indexes;
        // [Constraint 4: DISTANCE]
        for (int label = 0; label < number_of_variables; label++) {
            for (int new_label = label + 1; new_label < number_of_variables; new_label++) {

                // Find the respective points for each group
                vector<point> this_group;
                auto it = groups.find(label);
                if (it != groups.end()) {
                    this_group = it->second;
                }

                // Find the respective points for each group
                vector<point> this_new_group;
                auto it2 = groups.find(new_label);
                if (it2 != groups.end()) {
                    this_new_group = it2->second;
                }

                // Create the variables between the groups
                size_t index = variables.size();
                MPVariable* abs_gr_x1 = solver->MakeNumVar(0.0, +infinity, "abs_gr_x1");
                MPVariable* abs_gr_y2 = solver->MakeNumVar(0.0, +infinity, "abs_gr_y2");
                MPVariable* binary1 = solver->MakeBoolVar("bool1");
                MPVariable* binary2 = solver->MakeBoolVar("bool2");
                variables.push_back(abs_gr_x1);
                variables.push_back(abs_gr_y2);
                variables.push_back(binary1);
                variables.push_back(binary2);
                MPVariable* binary3 = solver->MakeBoolVar("bool3");
                MPVariable* binary4 = solver->MakeBoolVar("bool4");
                MPVariable* binary5 = solver->MakeBoolVar("bool5");
                MPVariable* binary6 = solver->MakeBoolVar("bool6");

                // Absolute between xs
                MPConstraint* const abs1_gr_x1 = solver->MakeRowConstraint(0, +infinity);
                abs1_gr_x1->SetCoefficient(variables[2 * label], 1);
                abs1_gr_x1->SetCoefficient(variables[2 * new_label], -1);
                abs1_gr_x1->SetCoefficient(variables[index], 1);
                MPConstraint* const abs2_gr_x1 = solver->MakeRowConstraint(0, +infinity);
                abs2_gr_x1->SetCoefficient(variables[2 * label], -1);
                abs2_gr_x1->SetCoefficient(variables[2 * new_label], 1);
                abs2_gr_x1->SetCoefficient(variables[index], 1);
                // Big M Constraints
                MPConstraint* const abs1_gr_Mx1 = solver->MakeRowConstraint(-infinity, 0);
                abs1_gr_Mx1->SetCoefficient(variables[2 * label], 1);
                abs1_gr_Mx1->SetCoefficient(variables[2 * new_label], -1);
                abs1_gr_Mx1->SetCoefficient(variables[index], 1);
                abs1_gr_Mx1->SetCoefficient(variables[index + 2], -M);
                MPConstraint* const abs2_gr_Mx1 = solver->MakeRowConstraint(-infinity, M);
                abs2_gr_Mx1->SetCoefficient(variables[2 * label], -1);
                abs2_gr_Mx1->SetCoefficient(variables[2 * new_label], 1);
                abs2_gr_Mx1->SetCoefficient(variables[index], 1);
                abs2_gr_Mx1->SetCoefficient(variables[index + 2], M);

                // Absolute between ys
                MPConstraint* const abs1_gr_y2 = solver->MakeRowConstraint(0, +infinity);
                abs1_gr_y2->SetCoefficient(variables[2 * label + 1], 1);
                abs1_gr_y2->SetCoefficient(variables[2 * new_label + 1], -1);
                abs1_gr_y2->SetCoefficient(variables[index + 1], 1);
                MPConstraint* const abs2_gr_y2 = solver->MakeRowConstraint(0, +infinity);
                abs2_gr_y2->SetCoefficient(variables[2 * label + 1], -1);
                abs2_gr_y2->SetCoefficient(variables[2 * new_label + 1], 1);
                abs2_gr_y2->SetCoefficient(variables[index + 1], 1);
                // Big M Constraints
                MPConstraint* const abs1_gr_My2 = solver->MakeRowConstraint(-infinity, 0);
                abs1_gr_My2->SetCoefficient(variables[2 * label + 1], 1);
                abs1_gr_My2->SetCoefficient(variables[2 * new_label + 1], -1);
                abs1_gr_My2->SetCoefficient(variables[index + 1], 1);
                abs1_gr_My2->SetCoefficient(variables[index + 3], -M);
                MPConstraint* const abs2_gr_My2 = solver->MakeRowConstraint(-infinity, M);
                abs2_gr_My2->SetCoefficient(variables[2 * label + 1], -1);
                abs2_gr_My2->SetCoefficient(variables[2 * new_label + 1], 1);
                abs2_gr_My2->SetCoefficient(variables[index + 1], 1);
                abs2_gr_My2->SetCoefficient(variables[index + 3], M);

                // They should also not be equal with each other
                MPConstraint* const not_eq_x1 = solver->MakeRowConstraint(-infinity, -epsilon);
                not_eq_x1->SetCoefficient(variables[2 * label], 1);
                not_eq_x1->SetCoefficient(variables[2 * new_label], -1);
                not_eq_x1->SetCoefficient(binary3, -M);

                MPConstraint* const not_eq_x2 = solver->MakeRowConstraint(-infinity, -epsilon + M);
                not_eq_x2->SetCoefficient(variables[2 * label], -1);
                not_eq_x2->SetCoefficient(variables[2 * new_label], 1);
                not_eq_x2->SetCoefficient(binary4, M);

                MPConstraint* const not_eq_y1 = solver->MakeRowConstraint(-infinity, -epsilon);
                not_eq_y1->SetCoefficient(variables[2 * label + 1], 1);
                not_eq_y1->SetCoefficient(variables[2 * new_label + 1], -1);
                not_eq_y1->SetCoefficient(binary5, -M);

                MPConstraint* const not_eq_y2 = solver->MakeRowConstraint(-infinity, -epsilon + M);
                not_eq_y2->SetCoefficient(variables[2 * label + 1], -1);
                not_eq_y2->SetCoefficient(variables[2 * new_label + 1], 1);
                not_eq_y2->SetCoefficient(binary6, M);

                // Find the maximum and minimum from each group
                double this_max = 0;
                double this_min = 100000;
                double this_new_max = 0;
                double this_new_min = 100000;

                for (int i = 0; i < this_group.size(); i++) {
                    point p1 = { this_group[i].x_coord, this_group[i].y_coord };
                    double value_min = dist_min_max[p1][0];
                    double value_max = dist_min_max[p1][1];
                    if (this_max < value_max) {
                        this_max = value_max;
                    }
                    if (this_min > value_min) {
                        this_min = value_min;
                    }
                }

                for (int j = 0; j < this_new_group.size(); j++) {
                    point p2 = { this_new_group[j].x_coord, this_new_group[j].y_coord };
                    double value_min = dist_min_max[p2][0];
                    double value_max = dist_min_max[p2][1];
                    if (this_new_max < value_max) {
                        this_new_max = value_max;
                    }
                    if (this_new_min > value_min) {
                        this_new_min = value_min;
                    }
                }
                /*
                cout << "This max " << this_max << endl;
                cout << "This min " << this_min << endl;
                cout << "This new max " << this_new_max << endl;
                cout << "This new min " << this_new_min << endl;*/

                double dif1 = (this_max - this_new_min) * c / ng;
                double dif2 = (this_new_max - this_min) * c / ng;
                double the_max_difference = max(dif1, dif2);
                double the_mean_difference = (dif1 + dif2) / 2;
                double adjustment_dist = the_max_difference - the_mean_difference;
                MPConstraint* const dist1 = solver->MakeRowConstraint(the_mean_difference - adjustment_dist - thres * c / ng, +infinity);
                dist1->SetCoefficient(variables[index], 1);
                dist1->SetCoefficient(variables[index + 1], 1);

                other_indexes.push_back(static_cast<int>(index));
            }
        } // [End Constraint 4: DISTANCE]
        if (debugger) {
            setTextColor(LIGHTGREEN);
            cout << "Distance constraints created successfully!" << endl;
            setTextColor(WHITE);
        }
        // [END constraints]

        // [START objective]
        if (debugger) {
            setTextColor(YELLOW);
            cout << "Commences objective function creation..." << endl;
            setTextColor(WHITE);
        }
        MPObjective* const objective = solver->MutableObjective();
        for (int i = 0; i < indexes.size(); i++) {
            int index = indexes[i];
            // Absolute value difference
            MPVariable* abs_diff = solver->MakeNumVar(0.0, +infinity, "abs_diff1");
            MPVariable* bin_diff = solver->MakeBoolVar("bool");
            MPConstraint* const abs_inter1 = solver->MakeRowConstraint(0, +infinity);
            abs_inter1->SetCoefficient(abs_diff, 1);
            abs_inter1->SetCoefficient(variables[index], -1);
            abs_inter1->SetCoefficient(variables[index + 1], -1);
            abs_inter1->SetCoefficient(variables[index + 2], 1);
            abs_inter1->SetCoefficient(variables[index + 3], 1);
            MPConstraint* const abs_inter2 = solver->MakeRowConstraint(0, +infinity);
            abs_inter2->SetCoefficient(abs_diff, 1);
            abs_inter2->SetCoefficient(variables[index], 1);
            abs_inter2->SetCoefficient(variables[index + 1], 1);
            abs_inter2->SetCoefficient(variables[index + 2], -1);
            abs_inter2->SetCoefficient(variables[index + 3], -1);
            MPConstraint* const abs_interM1 = solver->MakeRowConstraint(-infinity, 0);
            abs_interM1->SetCoefficient(abs_diff, 1);
            abs_interM1->SetCoefficient(variables[index], -1);
            abs_interM1->SetCoefficient(variables[index + 1], -1);
            abs_interM1->SetCoefficient(variables[index + 2], 1);
            abs_interM1->SetCoefficient(variables[index + 3], 1);
            abs_interM1->SetCoefficient(bin_diff, -M);
            MPConstraint* const abs_interM2 = solver->MakeRowConstraint(-infinity, M);
            abs_interM2->SetCoefficient(abs_diff, 1);
            abs_interM2->SetCoefficient(variables[index], 1);
            abs_interM2->SetCoefficient(variables[index + 1], 1);
            abs_interM2->SetCoefficient(variables[index + 2], -1);
            abs_interM2->SetCoefficient(variables[index + 3], -1);
            abs_interM2->SetCoefficient(bin_diff, M);
            objective->SetCoefficient(abs_diff, coeff);
        }
        for (int i = 0; i < other_indexes.size(); i++) {
            int index2 = other_indexes[i];
            objective->SetCoefficient(variables[index2], 1 - coeff);
            objective->SetCoefficient(variables[index2 + 1], 1 - coeff);
        }

        objective->SetMinimization();
        if (debugger) {
            setTextColor(LIGHTGREEN);
            cout << "Objective function created successfully!" << endl;
            setTextColor(WHITE);
            cout << "---\n";
        }
        // [END objective]

        // [START solve]
        const MPSolver::ResultStatus result_status = solver->Solve();
        // Check if the problem has an optimal solution.
        try {
            if (result_status != MPSolver::OPTIMAL) {
                throw runtime_error("|The problem does not have an optimal solution!|");
            }
        }
        catch (const runtime_error& error) {
            setTextColor(RED);
            cout << "|----------------------------------------------|" << endl;
            cout << error.what() << endl;
            cout << "|----------------------------------------------|" << endl;
            //Change the code so that the algorithm does not stop pre-emptively without solution
            exit(EXIT_FAILURE);
        }
        // [END solve]

        // [START print_solution]
        int num = 0;

        if (debugger) {
            setTextColor(GREEN);
            cout << "|---------------|" << endl;
            cout << "|Solution found!|\n";
            cout << "|---------------|" << endl;
            setTextColor(WHITE);
            cout << "---\n";
            cout << "* Optimal objective value = " << objective->Value() << "\n";
            cout << "* Number of variables used: " << variables.size() << endl;
            cout << "* Number of constraints = " << solver->NumConstraints() << endl;
            cout << "* Size of SRs = " << SRs.size() << endl;
            cout << "* Variables:\n";
            for (int i = 0; i < variables.size(); i++) {
                cout << variables[i]->name() << " = " << variables[i]->solution_value() << endl;
            }
            cout << "---\n";
            cout << "Locations of new MPs: \n";
            for (int i = 0; i < 2 * number_of_variables; i++) {
                cout << variables[i]->name() << " = " << variables[i]->solution_value() << endl;
            }
            cout << "---\n";
        }

        vector<point> group_points;
        for (int i = 0; i < 2 * number_of_variables; i++) {
            int bound = indexes[0];
            if (indexes[0] == 0) {
                bound = indexes[1];
            }
            if (i < bound) {
                if (i % 2 == 0) {
                    point p1 = { variables[i]->solution_value(), variables[i + 1]->solution_value() };
                    if (num < SRs.size()){
                        mapSRs[roundPointToPrecision(p1, precision)] = SRs[num];
                    }
                    num++;
                    group_points.push_back(p1);
                }
            }
        }
        // [END print_solution]

        return make_tuple(group_points, mapSRs);
    }
}  