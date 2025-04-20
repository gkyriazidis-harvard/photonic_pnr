// Given three collinear points p, q, r, the function checks if 
// point q lies on line segment 'pr' 
bool onSegment(point p, point q, point r)
{
    if (q.x_coord <= max(p.x_coord, r.x_coord) && q.x_coord >= min(p.x_coord, r.x_coord) &&
        q.y_coord <= max(p.y_coord, r.y_coord) && q.y_coord >= min(p.y_coord, r.y_coord))
        return true;

    return false;
}

// To find orientation of ordered triplet (p, q, r). 
// The function returns following values 
// 0 --> p, q and r are collinear 
// 1 --> Clockwise 
// 2 --> Counterclockwise 
int orientation(point p, point q, point r)
{
    double val = (q.y_coord - p.y_coord) * (r.x_coord - q.x_coord) -
        (q.x_coord - p.x_coord) * (r.y_coord - q.y_coord);

    if (val == 0) return 0;  // collinear 

    return (val > 0) ? 1 : 2; // clock or counterclock wise 
}

// The main function that returns true if line segment 'p1q1' 
// and 'p2q2' intersect. 
bool doIntersect(point p1, point q1, point p2, point q2)
{
    // Find the four orientations needed for general and 
    // special cases 
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    cout << o1 << o2 << o3 << o4 << endl;

    // General case 
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases 
    // p1, q1 and p2 are collinear and p2 lies on segment p1q1 
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and q2 are collinear and q2 lies on segment p1q1 
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are collinear and p1 lies on segment p2q2 
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

    // p2, q2 and q1 are collinear and q1 lies on segment p2q2 
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false; // Doesn't fall in any of the above cases 
}



vector<pair<point, point>> mapPoints(const std::vector<point>& team1, const std::vector<point>& team2) {
    // Check if the sizes of both teams are the same
    vector<pair<point, point>> lines;

    if (team1.size() != team2.size()) {
        std::cout << "Teams have different sizes. Cannot map points." << std::endl;
        return lines;
    }

    // Create a combined vector of points
    std::vector<point> combinedPoints;
    combinedPoints.reserve(team1.size() + team2.size());
    combinedPoints.insert(combinedPoints.end(), team1.begin(), team1.end());
    combinedPoints.insert(combinedPoints.end(), team2.begin(), team2.end());

    // Create an index vector for permutations
    std::vector<int> indices(combinedPoints.size());
    for (int i = 0; i < indices.size(); i++) {
        indices[i] = i;
    }


    // Generate permutations
    do {
        int check = 0;
        for (int i = 0; i < team1.size(); i++) {
            lines.push_back(make_pair(team1[i], combinedPoints[indices[i + team1.size()]]));
        }

        for (int i = 0; i < lines.size(); i++) {
            for (int j = i + 1; j < lines.size(); j++) {
                cout << lines[i].first << lines[i].second << lines[j].first << lines[j].second << endl;
                if (doIntersect(lines[i].first, lines[i].second, lines[j].first, lines[j].second)) {
                    check++;
                    cout << "They cross" << endl;
                }
            }
        }
        cout << check << endl;
        if (check == 0) {
            return lines;
        }
        else {
            lines.clear();
        }


    } while (std::next_permutation(indices.begin() + team1.size(), indices.end()));

    return lines;
}

