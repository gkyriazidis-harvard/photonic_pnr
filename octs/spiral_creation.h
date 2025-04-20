// This code contains the class definition of a waveguide spiral
// The ID of a spiral is (X)
// This code is under development
// -----------------------------------------------------------------------------------------------------------------------

static int max_num_layers = 20;
static double spacing = 0.1;

/*pair<int, int> connectSpiral(nary_tree_node* top, wgSpiral sprl, int bend_num, int wg_num) {

	vector<line> all_lines = sprl.get_spiral();
	line first = all_lines[0];
	int last_size = top->children.size();
	nary_tree_node* previous_child;
	point next_point = { -1 , -1 };

	if ((top->dev->print_loc1().x_coord == top->dev->print_loc2().x_coord && first.is_horizontal()) ||
		(top->dev->print_loc1().y_coord == top->dev->print_loc2().y_coord && first.is_vertical())) {
		//erase waveguide to create one that spans all the way
		//...
	}
	else {
		// Creates a bend and the first waveguide
		bend_num++;
		bend* bd = new bend(sprl.get_input(), 2, "Bend : " + to_string(bend_num));
		add_child(top, bd);
		previous_child = top->children[last_size];
		wg_num++;
		next_point = { sprl.get_input().x_coord + first.p2.x_coord, sprl.get_input().y_coord + first.p2.y_coord };
		waveguide* wg = new waveguide(sprl.get_input(), next_point, 1, "Waveguide : " + to_string(wg_num));
		add_child(previous_child, wg);
		previous_child = previous_child->children[0];
	}

	// Repeat until all the vector is finished
	for (size_t i = 1; i < all_lines.size(); i++) {
		line next_line = all_lines[i];
		bend_num++;
		bend* bd = new bend(next_point, 2, "Bend : " + to_string(bend_num));
		add_child(previous_child, bd);
		previous_child = previous_child->children[0];
		wg_num++;
		point previous_point = next_point;
		next_point = { previous_point.x_coord + next_line.p2.x_coord, previous_point.y_coord + next_line.p2.y_coord };
		waveguide* wg = new waveguide(previous_point, next_point, 1, "Waveguide : " + to_string(wg_num));
		add_child(previous_child, wg);
		previous_child = previous_child->children[0];
	}

	return make_pair(bend_num, wg_num);
}*/

//! Code needs adjustment
class wgSpiral : photo_device {
private:
	double width;
	double height;
	double length;
	int spiralLayers;
	point input;
	point output;
	int rot; //0 right, 1 up, 2 left, 3 down
	int mirrored; 
	vector<line> designed;
	area a_spir;

public:
	wgSpiral(double width, double length, int rot, int mirrored, point input, int id, string label) : photo_device(id, label) {
		this->width = width;
		this->length = length;
		this->rot = rot;
		this->mirrored = mirrored;
		this->input = input;
		pair<int, double> layerAndHeight = calc_layers_height(width, length);
		this->spiralLayers = layerAndHeight.first;
		this->height = layerAndHeight.second;
		// calculate the output
		this->designed = design_spiral();
	}

	vector<line> get_spiral() {
		return designed;
	}

	point get_input() {
		return input;
	}

	// Calculates the number of layers and the height by passing through all the possible number of layers, from 1 to max_num_layers
	pair<int, double> calc_layers_height(double width, double length) {

		unordered_map<int, double> layer2height;

		for (int n = 0; n < max_num_layers; n++) {
			// create the calculations and save based on the layers and the height
			// parameterise the spacing inside
			double sum = 0.0;
			for (int i = 0; i <= n; i++) {
				sum += i;
			}
			double final_sum = sum * 1.6;
			cout << final_sum << endl;
			double right_side = (length / 2) + (0.2 * (n + 1)) + (0.6 * (n + 1)) - (3 * width / 2) + (0.4 * (n + 1)) + final_sum - (2 * (n + 1) * width);
			double height = right_side / (2 * (n + 1) + 0.5);
			layer2height[n] = height;
		}

		double min_height = numeric_limits<double>::max();
		int layers = -1;

		for (const auto& lh : layer2height) {
			cout << lh.first + 1 << " & " << lh.second << endl;
			if (lh.second < min_height && (lh.first + 1) * 0.4 + 0.2 < abs(lh.second) && lh.second > 0 && (lh.first + 1) * 0.4 + 0.2 < width) {
				min_height = lh.second;
				layers = lh.first + 1;
			}
		}

		if (min_height == numeric_limits<double>::max()) {
			cerr << "Specific waveguide spiral cannot be created." << endl;
			abort();
		}

		return make_pair(layers, min_height);
	}
	
	// Area Calculator for the spiral
	double area_calculator() {
		return width * height;
	}

	// Virtual functions defined specifically for the spiral
	double print_length() {
		return length;
	}

	point print_loc1() {
		return input;
	}

	point print_loc2() {
		return output;
	}

	double print_width() {
		return width;
	}

	int print_rotation() {
		return rot;
	}

	// create a function that returns a vector of lines based on rotation and mirroring as well as input
	vector<line> design_spiral() {
		cout << spiralLayers << endl;
		cout << height << endl;
		vector<line> spiral;
		point first_point = input;
		double changing_width = width;
		double changing_height = height;
		point next_point;

		for (int i = 0; i < spiralLayers; i++) {
			if (rot == 0) {
				next_point = {first_point.x_coord + changing_width, first_point.y_coord};
				line l1 = { first_point, next_point };
				first_point = next_point;
				next_point = { first_point.x_coord, first_point.y_coord - changing_height };
				line l2 = { first_point, next_point };
				first_point = next_point;
				next_point = { first_point.x_coord - changing_width + 0.2, first_point.y_coord };
				line l3 = { first_point, next_point };
				first_point = next_point;
				next_point = { first_point.x_coord, first_point.y_coord + changing_height - 0.2 };
				line l4 = { first_point, next_point };
				first_point = next_point;
				changing_width -= 0.4;
				changing_height -= 0.4;
				spiral.push_back(l1);
				spiral.push_back(l2);
				spiral.push_back(l3);
				spiral.push_back(l4);
			}
		}

		// Intermediate part
		if (rot == 0) {
			changing_height += 0.1;
			next_point = { first_point.x_coord + changing_width, first_point.y_coord };
			line l1 = { first_point, next_point };
			first_point = next_point;
			next_point = { first_point.x_coord, first_point.y_coord - (changing_height / 2) };
			line l2 = { first_point, next_point };
			first_point = next_point;
			next_point = { first_point.x_coord - (changing_width - 0.1), first_point.y_coord };
			line l3 = { first_point, next_point };
			first_point = next_point;
			next_point = { first_point.x_coord, first_point.y_coord - (changing_height / 2) };
			line l4 = { first_point, next_point };
			first_point = next_point;
			next_point = { first_point.x_coord + changing_width, first_point.y_coord };
			line l5 = { first_point, next_point };
			first_point = next_point;
			spiral.push_back(l1);
			spiral.push_back(l2);
			spiral.push_back(l3);
			spiral.push_back(l4);
			spiral.push_back(l5);
		}

		// Rest of the spiral
		for (int i = 0; i < spiralLayers; i++) {
			if (rot == 0) {
				next_point = { first_point.x_coord, first_point.y_coord + changing_height + 0.1 };
				line l1 = { first_point, next_point };
				first_point = next_point;
				next_point = { first_point.x_coord - changing_width - 0.2, first_point.y_coord };
				line l2 = { first_point, next_point };
				first_point = next_point;
				next_point = { first_point.x_coord, first_point.y_coord - changing_height - 0.3 };
				line l3 = { first_point, next_point };
				first_point = next_point;
				next_point = { first_point.x_coord + changing_width + 0.4, first_point.y_coord };
				line l4 = { first_point, next_point };
				first_point = next_point;
				changing_width += 0.4;
				changing_height += 0.4;
				spiral.push_back(l1);
				spiral.push_back(l2);
				spiral.push_back(l3);
				spiral.push_back(l4);
			}
		}

		return spiral;
	}

	// create a function that returns the power
	double calc_power() {
		double power_wg = length * waveguide_prop_loss;
		double power_bd = (designed.size() - 1) * bend_ins_loss;
		return power_wg + power_bd;
	}

	// Function that returns the delay 
	double calc_delay() {
		double delay = length / vg;
		return delay;
	}
};