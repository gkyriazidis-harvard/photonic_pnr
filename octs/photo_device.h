// This code contains the main class definition of a photonic device, which is the parent class for every other device
// -----------------------------------------------------------------------------------------------------------------------

// Base class representing all photonic devices
class photo_device {

public:
    int object_id;
    string label;
    vector<photo_device*> connections;

    //Photonic device constructor
    photo_device(int object_id, string label) {
        this->object_id = object_id;
        this->label = label;
    }

    // Adds a connection with another photonic device
    void addConnection(photo_device* obj) {
        connections.push_back(obj);
    }

    // Pure virtual functions to extract info
    virtual double print_length() { 
        return -1; 
    };

    virtual point print_loc1() {
        point p1 = { -1,-1 };
        return p1;
    };

    virtual point print_loc2() {
        point p1 = { -1,-1 };
        return p1;
    };

    virtual double print_width() {
        return -1;
    };

    virtual int print_rotation() {
        return -1;
    };

    virtual double calc_loss() {
        return -1;
    };

    virtual double calc_delay() {
        return 0;
    };

    virtual double calc_length() {
        return -1;
    };

    virtual double get_sr() {
        return -1;
    }
};