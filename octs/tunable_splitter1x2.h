// This code contains the class definition of a 1x2 tunable (DC) splitter
// The ID of a 1x2 tunable (DC) splitter is (7)
// -----------------------------------------------------------------------------------------------------------------------

class tunable_splitter1x2 : public photo_device {

private:

    point xy_position;
    double SR;
    point path_sr1; //necessary to recognise to which path the SR refers to
    point path_sr2; //necessary to recognise to which path the SR refers to

public:

    // Empty constructor    
    tunable_splitter1x2() : photo_device(-1, "") {
        xy_position.x_coord = -1;
        xy_position.y_coord = -1;
        SR = 0.5;
        path_sr1 = point(-1, -1);
        path_sr2 = point(-1, -1);
    }

    // Constructor
    tunable_splitter1x2(point xy_position, int id, string label) : photo_device(id, label) {
        this->xy_position.x_coord = xy_position.x_coord;
        this->xy_position.y_coord = xy_position.y_coord;
    }

    // Loss Calculation 
    double calc_loss() override {
        double losses = splitter_ins_loss_1x2;
        return losses;
    }

    // Functions for accessing through a connection
    point print_loc1() override {
        return xy_position;
    }

    // Functions for setting the variables
    void set_xy_position(point xy_position) {
        this->xy_position = xy_position;
    }

    // Function to set the SR of the tunable splitter
    void set_sr(double SR) {
        this->SR = SR;
    }

    // Function that returns the custom splitting ratio of the tunable splitter
    double get_sr() {
        return SR;
    }

    point get_path_sr1() {
        return path_sr1;
    }

    void set_path_sr1(point p1) {
        path_sr1 = p1;
    }

    point get_path_sr2() {
        return path_sr2;
    }

    void set_path_sr2(point p1) {
        path_sr2 = p1;
    }
};