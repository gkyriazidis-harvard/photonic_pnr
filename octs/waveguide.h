// This code contains the class definition of a waveguide
// The ID of a waveguide is (1)
// -----------------------------------------------------------------------------------------------------------------------


class waveguide : public photo_device {

private:

    point xy_start;
    point xy_end;
    double length;

public:

    // Empty constructor
    waveguide() : photo_device(-1, "") {
        xy_start.x_coord = -1;
        xy_start.y_coord = -1;
        xy_end.x_coord = -1;
        xy_end.y_coord = -1;
        length = -1;
    }

    // Constructor
    waveguide(point xy_start, point xy_end, int id, string label) : photo_device(id, label) {
        this->xy_start.x_coord = xy_start.x_coord;
        this->xy_start.y_coord = xy_start.y_coord;
        this->xy_end.x_coord = xy_end.x_coord;
        this->xy_end.y_coord = xy_end.y_coord;
        this->length = calc_length();
    }

    // Delay Calculation 
    double calc_delay() {
        double delay = length / vg;
        return delay;
    }

    // Loss Calculation 
    double calc_loss() override {
        double losses = length * waveguide_prop_loss;
        return losses;
    }

    // Length Calculation 
    double calc_length() override {
        double len = abs(xy_start.x_coord - xy_end.x_coord) + abs(xy_start.y_coord - xy_end.y_coord);
        return len;
    }

    // Functions for accessing through a connection
    double print_length() override{
        return length;
    }

    point print_loc1() override {
        return xy_start;
    }

    point print_loc2() override {
        return xy_end;
    }

    // Functions for setting the variables
    void set_length() {
        this->length = calc_length();
    }

    void set_xy_start(point xy_start) {
        this->xy_start = xy_start;
    }

    void set_xy_end(point xy_end) {
        this->xy_end = xy_end;
    }
};
