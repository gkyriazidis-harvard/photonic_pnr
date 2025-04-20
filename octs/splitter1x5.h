// This code contains the class definition of a 1x5 splitter
// The ID of a 1x5 splitter is (6)
// -----------------------------------------------------------------------------------------------------------------------

class splitter1x5 : public photo_device {

private:

    point xy_position; // position of the bottom point
    double width; // width is the x axis
    double length; // length is the y axis 
    int rotation; // 0, 90, 180, 270 degrees
    //area ar1x5;

public:

    // Empty constructor    
    splitter1x5() : photo_device(-1, "") {
        xy_position.x_coord = -1;
        xy_position.y_coord = -1;
        width = -1;
        length = -1;
        rotation = -1;
    }

    // Constructor
    splitter1x5(point xy_position, int id, string label, double width, double length, int rotation) : photo_device(id, label) {
        this->xy_position.x_coord = xy_position.x_coord;
        this->xy_position.y_coord = xy_position.y_coord;
        this->width = width;
        this->length = length;
        this->rotation = rotation;
    }

    // Loss Calculation 
    double calc_loss() override {
        double losses = splitter_ins_loss_1x5;
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

    double print_width() override {
        return width;
    };

    int print_rotation() override {
        return rotation;
    }

    double print_length() override  {
        return length;
    };

    /*
    void set_area(point& bl) {
        this->ar1x5.setValues(bl, length_spl1x3, width_spl1x3, keepout, -1);
    }*/

};