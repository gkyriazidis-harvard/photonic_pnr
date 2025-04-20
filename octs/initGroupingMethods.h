// This code contains the cluster initialization methods implemented and tested on the algorithm
// These include maximin - kaufmann - kmeanspp
// There is no method that is better than the others as it depends on the sink number and locations
// -----------------------------------------------------------------------------------------------------------------------

void maximinInitCentroids(const vector<point>& points, unordered_multimap<point, vector<int>, pointHash>& centroids, const vector<vector<int>>& constraints) {
    /**
    * Maximin initialization method of centroids
    * Deterministic Method
    * 
    * @param points The points corresponding to the sink/MP locations to be added in clusters
    * @param centroids The map connecting a constraint with each cluster
    * @param constraints Vector of constraints created from topology vector
    */
    // Step 1:
    // Calculate the Euclidean norm for all points
    double max_norm = numeric_limits<double>::min();
    point max_norm_point;

    for (const auto& pt : points) {
        double norm = sqrt(pow(pt.x_coord, 2) + pow(pt.y_coord, 2));
        if (norm > max_norm) {
            max_norm = norm;
            max_norm_point = pt;
        }
    }

    centroids.insert(make_pair(max_norm_point, constraints[0]));

    // Step 2:
    // Find the next centroids based on distance
    for (int j = 1; j < constraints.size(); ++j) {
        unordered_map<int, double> all_dists;
        for (int k = 0; k < points.size(); k++) {
            // If the point exists in the centroid, then ignore it
            if (centroids.find(points[k]) != centroids.end()) {
                continue;
            }

            // From all the centroids find the one that is closer to this specific k point
            double min_dist = numeric_limits<double>::max();

            for (const auto& cen : centroids) {
                double dist = euclideanDistance(cen.first, points[k]);
                if (dist < min_dist) {
                    min_dist = dist;
                }
            }
            all_dists[k] = min_dist;
        }

        // Find the centroid through the all_dists map
        double max_dist = numeric_limits<double>::min();
        int max_key = -1;

        for (const auto& [key, value] : all_dists) {
            if (value > max_dist) {
                max_dist = value;
                max_key = key;
            }
        }

        centroids.insert(make_pair(points[max_key], constraints[j]));
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void kaufmanInitCentroids(const vector<point>& points, unordered_multimap<point, vector<int>, pointHash>& centroids, const vector<vector<int>>& constraints) {
    /**
    * Kaufmann initialization method of centroids
    * Deterministic Method
    * 
    * @param points The points corresponding to the sink/MP locations to be added in clusters
    * @param centroids The map connecting a constraint with each cluster
    * @param constraints Vector of constraints created from topology vector
    */
    // Step 1:
    // Select the closest data point to the global centroid of the dataset
    double sumx = 0;
    double sumy = 0;

    for (const auto& pt : points) {
        sumx += pt.x_coord;
        sumy += pt.y_coord;
    }

    point centroid = { sumx / points.size(), sumy / points.size() };

    // Initialize closest point
    point closest = points[0];
    double min_dist = euclideanDistance(centroid, points[0]);

    for (const auto& pt : points) {
        double dist = euclideanDistance(centroid, pt);
        if (dist < min_dist) {
            closest = pt;
            min_dist = dist;
        }
    }

    // Load the closest point as the first centroid
    centroids.insert(make_pair(closest, constraints[0]));

    // Step 2:
    // For every two non selected data points calculate C
    for (int j = 1; j < constraints.size(); ++j) {
        map<int, double> all_c_sums;
        for (int k = 0; k < points.size(); k++) {
            // If the point exists in the centroid, then ignore it
            if (centroids.find(points[k]) != centroids.end()) {
                continue;
            }

            double c_sum = 0;

            // From all the centroids find the one that is closer to this specific k point
            double min_dist = numeric_limits<double>::max();

            for (const auto& cen : centroids) {
                double dist = euclideanDistance(cen.first, points[k]);
                if (dist < min_dist) {
                    min_dist = dist;
                }
            }

            for (int l = 0; l < points.size(); l++) {
                // If the point exists in the centroid, then ignore it
                if (centroids.find(points[l]) != centroids.end() || k == l) {
                    continue;
                }
                double c_value = max(min_dist - euclideanDistance(points[k], points[l]), 0.0);
                c_sum += c_value;
            }
            all_c_sums[k] = c_sum;
        }

        // Find the centroid through the all_c_sums map
        double min_sum = numeric_limits<double>::max();
        int min_key = -1;

        for (const auto& [key, value] : all_c_sums) {
            if (value < min_sum) {
                min_sum = value;
                min_key = key;
            }
        }

        centroids.insert(make_pair(points[min_key], constraints[j]));
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void kmeansppInitCentroids(const vector<point>& points, unordered_multimap<point, vector<int>, pointHash>& centroids, const vector<vector<int>>& constraints) {
    /**
    * Function that uses kmeans++ algorithm to initialize the centroids
    * Stochastic Method so the result may be different for different runs, though multiple trials can be run
    * 
    * @param points The points corresponding to the sink/MP locations to be added in clusters
    * @param centroids The map connecting a constraint with each cluster
    * @param constraints Vector of constraints created from topology vector
    */
    vector<bool> assigned(points.size(), false);
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(0.0, 1.0);
    vector<point> centers;

    vector<double> chances(points.size(), 1.0);

    for (int j = 0; j < constraints.size(); ++j) {
        // Normalize chances
        double sum_chances = accumulate(chances.begin(), chances.end(), 0.0);
        for (size_t i = 0; i < chances.size(); ++i) {
            chances[i] /= sum_chances;
        }

        // Choose a center based on chances
        double r = dist(gen);
        double acc = 0.0;
        int index = 0;
        for (; index < chances.size(); ++index) {
            if (acc + chances[index] >= r) {
                break;
            }
            acc += chances[index];
        }
        centers.push_back(points[index]);

        centroids.insert(make_pair(points[index], constraints[j]));

        assigned[index] = true;

        // Update chances based on distances to new center
        for (size_t i = 0; i < points.size(); ++i) {
            if (assigned[i]) {
                continue;
            }
            auto sc = sortedCentroids(centers, points[i]);
            vector<int> indices = get<0>(sc);
            vector<double> dists = get<1>(sc);
            chances[i] = dists[indices[0]];
        }
    }
}

// -----------------------------------------------------------------------------------------------------------------------