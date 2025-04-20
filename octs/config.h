// Configuration file
// -----------------------------------------------------------------------------------------------------------------------

// Keepout static variable for the rectangular areas, equal to 0.1 mm by default.
static double keepout = 0.1;

// Interposer dimensions (mm)
static double dim_x = 45; 
static double dim_y = 45; 

// Group refractive index LiNb
static double ng = 2.2; 

// Speed of light in LiNb (mm per ps)
static double c = 0.299792458; 
static double vg = c / ng; 

// Insertion and propagation losses for photonic components (dB per mm & dB)
static double waveguide_prop_loss = 0.01;
static double bend_ins_loss = 0.1;
static double cross_ins_loss = 0.5; 
static double splitter_ins_loss_1x2 = 1; 
static double splitter_ins_loss_1x3 = 1; 
static double splitter_ins_loss_1x5 = 1;

// Skew threshold (ps)
static double thres = 10; 
// Power threshold (dB)
static double power_thres = 3;

// Dimensions of several photonic components (mm)
static double width_spl1x3 = 0.075; 
static double width_spl1x5 = 0.075; 
static double length_spl1x3 = 0.4; 
static double length_spl1x5 = 0.4; 
static double length_wg_spl1x3 = 0.1; // lengths of the short waveguide at the end of the 1x3 or 1x5 splitter
static double length_wg_spl1x5 = 0.1; 
static double bend_radius = 0.05;

// Coefficient that controls the optimization target // 1 -> minimize the extra skew inserted, 0 -> minimize the distances between the MPs
static double coeff = 0.5; 

// Debugger value, if set to true allows certain couts for debugging purposes
static bool debugger = true;
// Timer value used to calculate the time throughout different stages of the algorithm
static bool timer = true;

// Input and output filenames
static string input_filename = "./Sinks/r1_4_1.txt";
static string output_filename = "./Results/r1_4_1.txt";
static string output_filename_verilog = "./Results/r1_4_1.v";


//! Do not change the values below
// Static numbers used for optimization
static const double M = 100000;
static const double epsilon = 0.001;
// Precision selected for the double results
static int precision = 10;
// Variable that controls the expansion of the search window during routing, equal to 0.1 mm by default (routing)
static double windowExpansion = 0.1;
// Support of splitters with this many outputs 
// So far code is hardcoded to support 1x2, 1x3, 1x5 splitters and 1x2 DC for tunable splitter
static vector<int> splitter_avail = { 2, 3, 5 };
static vector<int> heater_avail = { 2 };
// Constant values necessary to define colors for terminal outputs
const int BLACK = 0;
const int BLUE = 1;
const int GREEN = 2;
const int CYAN = 3;
const int RED = 4;
const int MAGENTA = 5;
const int BROWN = 6;
const int LIGHTGRAY = 7;
const int DARKGRAY = 8;
const int LIGHTBLUE = 9;
const int LIGHTGREEN = 10;
const int LIGHTCYAN = 11;
const int LIGHTRED = 12;
const int LIGHTMAGENTA = 13;
const int YELLOW = 14;
const int WHITE = 15;