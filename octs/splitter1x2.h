// This code contains the class definition of a 1x2 splitter
// The ID of a 1x2 splitter is (2)
// -----------------------------------------------------------------------------------------------------------------------

class splitter1x2 : public photo_device {

private:

    point xy_position;

public:

    // Empty constructor    
    splitter1x2() : photo_device(-1, "") {
        xy_position.x_coord = -1;
        xy_position.y_coord = -1;
    }

    // Constructor
    splitter1x2(point xy_position, int id, string label) : photo_device(id, label) {
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
};