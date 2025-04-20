// This code contains the class definition of a 1x3 splitter
// The ID of a 1x3 splitter is (5)
// -----------------------------------------------------------------------------------------------------------------------

class splitter1x3 : public photo_device {

private:

    point xy_position;
    double width; // width is the X axis
    double length; // length is the Y axis 
    int rotation; // 0, 90, 180, 270 degrees
    //area ar1x3;

public:

    // Empty constructor    
    splitter1x3() : photo_device(-1, "") {
        xy_position.x_coord = -1;
        xy_position.y_coord = -1;
        width = -1;
        length = -1;
        rotation = -1;
    }

    // Constructor
    splitter1x3(point xy_position, int id, string label, double width, double length, int rotation) : photo_device(id, label) {
        this->xy_position.x_coord = xy_position.x_coord;
        this->xy_position.y_coord = xy_position.y_coord;
        this->width = width;
        this->length = length;
        this->rotation = rotation;
    }

    // Loss Calculation 
    double calc_loss() override {
        double losses = splitter_ins_loss_1x3;
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

    double print_length() override {
        return length;
    };

    int print_rotation() override {
        return rotation;
    }

    /*
    void set_area(point bl) {
        this->ar1x3.setValues(bl, length_spl1x3, width_spl1x3,keepout, -1);
    }*/

};


