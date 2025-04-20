// This code contains the grouping method that is COP-KMeans (Constrained K-Means Algorithm)
// The initialization method can be chosen by the user, default is Maximin
// -----------------------------------------------------------------------------------------------------------------------

unordered_map<int, vector<point>> copKMeansClustering(unordered_map<point, vector<int>, pointHash> points, vector<vector<int>>& constraints, init_method method, int max_iter=2000) {

    // Step 1: Initialize cluster centroids based on must-link constraints
    unordered_multimap<point, vector<int>, pointHash> centroids; // a map of the centroids and the constraints
    unordered_map<point, bool, pointHash> assigned; // shows if the point has been assigned to a cluster or not
    vector<point> collection; // collection of points as input to the initialization function
    size_t k = constraints.size();

    for (const auto& pair : points) {
        assigned[pair.first] = false;
        collection.push_back(pair.first);
    }

    double tolerance = 10e-5;

    switch (method) {
    case init_method::KAUFMAN:
        kaufmanInitCentroids(collection, centroids, constraints);
        break;
    case init_method::KMEANSPP:
        kmeansppInitCentroids(collection, centroids, constraints);
        break;
    default: 
        maximinInitCentroids(collection, centroids, constraints);
        break;
    }
    // Step 1 : Finished

    // Step 2: Assign points to clusters iteratively while respecting constraints
    vector<point> prev_centroids;
    vector<vector<point>> every_group;
    vector<point> all;
    vector<double> all_distances;
    double prev_sum_of_squares_x = 0;
    double prev_sum_of_squares_y = 0;
    unordered_multimap<point, vector<int>, pointHash> centroidsConst = centroids;

    for (int i = 0; i < max_iter; i++) {
        centroids = centroidsConst;
        prev_centroids.clear();
        unordered_multimap<point, vector<point>, pointHash> centroidToMPs;
        every_group.clear();
        for (const auto& centers : centroids) {
            prev_centroids.push_back(centers.first);
            centroidToMPs.insert(make_pair(centers.first, vector<point>()));
        }
        for (const auto& pair : points) {
            assigned[pair.first] = false;
        }
        for (const auto& pair : points) {

            // Find the closest centroids for a specific point
            tuple<vector<int>, vector<double>> closest = sortedCentroids(prev_centroids, pair.first);
            vector<int> indices = get<0>(closest);

            int count = 0;
            if (assigned[pair.first] == false) {
                bool found = false;
                while (!found && count < indices.size()) {
                    int index = indices[count];
                    auto ex_range = centroids.equal_range(prev_centroids[index]);
                    for (auto ex_it = ex_range.first; ex_it != ex_range.second; ++ex_it) {
                        if (!constraintViolationChecker(ex_it->second, points[pair.first][3])) {
                            found = true;
                            assigned[pair.first] = true;
                            auto range = centroidToMPs.equal_range(prev_centroids[index]);
                            for (auto it = range.first; it != range.second; ++it) {
                                it->second.push_back(pair.first);
                            }
                        }
                        if (found == true) {
                            break;
                        }
                    }
                    count++;
                }
                if (!found) {
                    cerr << "No available cluster found." << endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        for (const auto& centroid : centroidToMPs) {
            for (const auto& mp : centroid.second) {
                all.push_back(mp);
            }
            every_group.push_back(all);
            all.clear();
        }

        // Update the centroids based on the current assignments
        vector<point> newCentroids(k);
        unordered_multimap<point, vector<int>, pointHash> new_centroids_map;
        vector<int> clusterCount(k, 0);

        int group = 0;
        for (const vector<point>& points : every_group) {
            for (const point& point : points) {
                newCentroids[group].x_coord += point.x_coord;
                newCentroids[group].y_coord += point.y_coord;
                ++clusterCount[group];
            }
            group++;
        }

        double total_sum = 0;

        for (int j = 0; j < k; ++j) {

            if (clusterCount[j] > 0) {
                newCentroids[j].x_coord /= clusterCount[j];
                newCentroids[j].y_coord /= clusterCount[j];
            } 
            auto this_range = centroidsConst.equal_range(prev_centroids[j]);
            int num = 0;
            for (auto it = this_range.first; it != this_range.second; ++it) {
                num++;
                if (num > 1) {
                    j++;
                    if (clusterCount[j] > 0) {
                        newCentroids[j].x_coord /= clusterCount[j];
                        newCentroids[j].y_coord /= clusterCount[j];
                    }
                }
                new_centroids_map.insert(make_pair(newCentroids[j], it->second));
            }
            total_sum += sqrt(pow(abs(newCentroids[j].x_coord - prev_centroids[j].x_coord), 2) + pow(abs(newCentroids[j].y_coord - prev_centroids[j].y_coord), 2));
        }

        if (total_sum < tolerance) {
            break;
        }

        centroidsConst = new_centroids_map;
    }

    return convertToMap(every_group);
}

// -----------------------------------------------------------------------------------------------------------------------

unordered_map<int, vector<point>> topCopKMeansClustering(unordered_map<point, vector<int>, pointHash> points, vector<vector<int>>& constraints, init_method method=init_method::MAXIMIN) {
    /**
    * Function that runs Constrained KMeans Clustering Algorithm
    * 
    * @param points Map consisting of the points (sinks/MPs) and their number of devices each one corresponds to 
    * @param constraints The constraints vector from the topology vector
    * @param method,OPTIONAL Initialization method, by default chosen as MAXIMIN
    */
    unordered_map<int, vector<point>> every_group;
    unordered_map<int, vector<point>> best_group;

    double total_dist = 0;
    double min_dist = numeric_limits<double>::max();
    size_t num_runs = 0;

    // If it is stochastic then run for the range of the points
    switch (method) {
    case init_method::KMEANSPP:
        // This number can be adjusted
        num_runs = constraints.size(); 
        break;
    default:
        num_runs = 1;
        break;
    }

    // Selects the best grouping result based on the total distance between the centroids
    //! The deterministic approaches do not guarantee selecting the optimal solution
    for (int init = 0; init < num_runs; init++) {

        every_group = copKMeansClustering(points, constraints, method);

        for (const auto& points : every_group) {

            double midx = 0;
            double midy = 0;

            for (const auto& pointy : points.second) {
                midx += pointy.x_coord;
                midy += pointy.y_coord;
            }

            midx /= points.second.size();
            midy /= points.second.size();

            point mid = { midx, midy };

            for (int k = 0; k < points.second.size(); k++) {
                total_dist += euclideanDistance(points.second[k], mid);
            }

        }

        if (min_dist > total_dist) {
            min_dist = total_dist;
            best_group = every_group;
        }

        total_dist = 0;
    }

    return best_group;
}

// -----------------------------------------------------------------------------------------------------------------------