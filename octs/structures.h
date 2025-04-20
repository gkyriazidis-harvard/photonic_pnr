// This code contains several important structures
// -----------------------------------------------------------------------------------------------------------------------

enum class init_method {
    /**
    * Initialization method that can be chosen for grouping algorithm
    */
    KAUFMAN,
    KMEANSPP,
    MAXIMIN
};

// -----------------------------------------------------------------------------------------------------------------------

struct point {
    /**
    * Point structure (in the 2D plane) that consists of two coordinates
    */
    double x_coord;
    double y_coord;

    bool operator==(const point& other) const {
        /**
        * Two points are equal only if both of their coordinates are equal 
        */
        return x_coord == other.x_coord && y_coord == other.y_coord;
    }

    bool operator!=(const point& other) const {
        /**
        * Two points are not equal if either of their coordinates are not equal
        */
        return x_coord != other.x_coord || y_coord != other.y_coord;
    }

    point operator-(const double& number) const {
        /**
        * Subtracting a point with a number means subtracting each coordinate with the same number
        */
        return point{ x_coord - number, y_coord - number };
    }

    point operator+(const double& number) const {
        /**
        * Adding a point with a number means adding each coordinate with the same number
        */
        return point{ x_coord + number, y_coord + number };
    }

    bool operator<(const point& other) const {
        if (x_coord < other.x_coord) {
            return true;
        }
        else if (x_coord == other.x_coord && y_coord < other.y_coord) {
            return true;
        }
        return false;
    }
};

// -----------------------------------------------------------------------------------------------------------------------

struct line {
    /**
    * Line structure constructed by two points
    */

    point p1;
    point p2;


    bool is_vertical() {
        /**
        * Code that checks if a line is vertical
        */
        return (p1.x_coord == p2.x_coord);
    }

    bool is_horizontal() {
        /**
        * Code that checks if a line is horizontal
        */
        return (p1.y_coord == p2.y_coord);
    }
};

// -----------------------------------------------------------------------------------------------------------------------

ostream& operator<<(ostream& os, const point& other) {
    /**
    * Point printer function
    */
    os << "(" << other.x_coord << ", " << other.y_coord << ")";
    return os;
}

// -----------------------------------------------------------------------------------------------------------------------

ostream& operator<<(ostream& os, const line& l) {
    /**
    * Line printer function
    */
    os << l.p1 << " to " << l.p2;
    return os;
}

// -----------------------------------------------------------------------------------------------------------------------

struct pointHash {
    /**
    * Hash point structure for using the points as keys in maps.
    */
    size_t operator()(const point& p) const {
        // Combine the hash values of x and y using bitwise XOR
        return hash<double>()(p.x_coord) ^ hash<double>()(p.y_coord);
    }
};

// -----------------------------------------------------------------------------------------------------------------------

struct NodeHash {
    /**
    * Hash node structure for using the nodes as keys in maps.
    */
    size_t operator()(const point& pos) const {
        return std::hash<double>{}(pos.x_coord) ^ std::hash<double>{}(pos.y_coord);
    }
};

// -----------------------------------------------------------------------------------------------------------------------

struct Node {
    /**
    * Node structure necessary for the routing stage.
    */
    point p; // location of node
    int i, j; 
    unordered_map<Node*, vector<pair<Node*, double>>> neighbors;
    vector<Node*> prev_conns;
};

// -----------------------------------------------------------------------------------------------------------------------

struct NodeComparator {
    /**
    * Node comparator compares two pairs based on their distance cost which is the first element.
    */
    bool operator()(const pair<double, Node*>& a, const pair<double, Node*>& b) {
        return a.first > b.first; 
    }
};

// -----------------------------------------------------------------------------------------------------------------------

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& vec) {
    /**
     * Function used to print a vector and its content
     * * The content of the vector should also be printable
     * @param os The output stream
     * @param vec The vector to be printed
     */
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i < vec.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

// -----------------------------------------------------------------------------------------------------------------------

// Rectangular area structure defined by the bottom left point, the height and the width.
// This code is used to account for overlaps of waveguides with areas

struct area {

    point bl;
    double height;
    double width;
    point ur;
    point ul;
    point br;
    bool kp;
    //clock wise which edges have a keepout, necessary for the input/output edges // viable values 0, 1, 2, 3 based on rotation
    int edge;

    void setValues(point& p, double h, double w, bool kp, int edge) {
        if (kp) {
            if (edge == 0 || edge == 2) {
                bl = point{ p.x_coord - keepout, p.y_coord };
                height = h;
                width = w + 2 * keepout;
            }
            else if (edge == 1 || edge == 3) {
                bl = point{ p.x_coord, p.y_coord - keepout };
                height = h + 2 * keepout;
                width = w;
            }
            else {
                bl = point{ p.x_coord - keepout, p.y_coord - keepout };
                height = h + 2 * keepout;
                width = w + 2 * keepout;
            }
        }
        else {
            bl = p;
            height = h;
            width = w;
        }
        ur = point{ bl.x_coord + width, bl.y_coord + height };
        ul = point{ bl.x_coord, ur.y_coord };
        br = point{ ur.x_coord, bl.y_coord };
    }
};