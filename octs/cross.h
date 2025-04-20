// This code contains the class definition of a cross
// The ID of a cross is (3)
// -----------------------------------------------------------------------------------------------------------------------

class cross : public photo_device {

private:

    point xy_position;

public:

    // Empty constructor
    cross() : photo_device(-1, "") {
        xy_position.x_coord = -1;
        xy_position.y_coord = -1;
    }

    // Constructor
    cross(point xy_position, int id, string label) : photo_device(id, label) {
        this->xy_position.x_coord = xy_position.x_coord;
        this->xy_position.y_coord = xy_position.y_coord;
    }

    // Loss Calculation 
    double calc_loss() override {
        double losses = cross_ins_loss;
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

};