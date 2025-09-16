const int BTN_UP_PIN = 1;  // Button to increase the selected flutter option
const int BTN_DW_PIN = 0;  // Button to decrease the selected flutter option

#define NUM_CHANNELS 15
//                                   1
const int pwmPins[NUM_CHANNELS] = { 27,  12,   8,  19,  10,  15,  14,  13,  18,   9,  21,   7,  20,  22,  17}; // PWM-capable pins
const int ledWls[NUM_CHANNELS]  = {365, 373, 393, 404, 425, 429, 445, 466, 489, 514, 539, 594, 628, 661, 680}; // WLS

const int NUM_SLOTS = 9; 
int pwmBank[NUM_SLOTS][NUM_CHANNELS] = {
//  307,  308,  309,  310,  311,  312,  313,  314,  315,  317,  318,  319,  320,  321,  322
  {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  { 7236,  6897,  4678,  4578,  6432,  1097,  5784,  7325, 12635,  7101, 27726,     0,   476,   612,  1081},
  { 4127,  3274,  2474,  4191,  3769,  1174,  4775,  5926,  8304,  7930, 27726,     0,     0,    40,   285},
  { 3343,  3058,  2463,  4626,  4451,  1408,  6112,  7813, 12517,  7434, 27726,     0,   211,   428,   688},
  { 9465,  7859,  6024, 10125,  9326,  2775, 12101, 14165, 21029, 13301, 27726,     0,   793,  1269,  2180},
  {30946, 27366, 14522, 12825, 16395,  2908, 14538, 17880, 29249, 16105, 65535,     0,  1182,  1451,  2505},
  {17650, 12993,  7681, 11741,  9607,  3112, 12002, 14466, 19224, 17985, 65535,     0,     0,    94,   660},
  {14297, 12135,  7646, 12959, 11347,  3734, 15361, 19072, 28976, 16859, 65535,     0,   523,  1015,  1593},
  {40480, 31185, 18702, 28362, 23771,  7357, 30415, 34575, 48681, 30165, 65535,     0,  1968,  3008,  5051}
};

// flutterParams holds the control values for switching between pwmBanks with a particular transition
// Position 0: light setting 1
// Position 1: light setting 2
// Position 2: frequency [Hz]
// Position 3: duty cycle
// Position 4: part of duty cycle for transitions
// Position 5: type of transition (0: linear, 1: sine)
const int NUM_OPTIONS = 13;
float flutterParams[NUM_OPTIONS][6] = {
    { 0,     0,      1.0,     75.0,      20.0,       1},
    { 0,     1,      1.0,     75.0,      20.0,       1},
    { 0,     2,      1.0,     75.0,      20.0,       1},
    { 0,     3,      1.0,     75.0,      20.0,       1},
    { 0,     4,      1.0,     75.0,      20.0,       1},
    { 0,     5,      1.0,     75.0,      20.0,       1},
    { 0,     6,      1.0,     75.0,      20.0,       1},
    { 0,     7,      1.0,     75.0,      20.0,       1},
    { 0,     8,      1.0,     75.0,      20.0,       1},
    { 0,     2,      2.0,     75.0,      20.0,       1},
    { 0,     2,      4.0,     75.0,      20.0,       1},
    { 0,     2,      8.0,     75.0,      20.0,       1},
    { 2,     2,      1.0,     75.0,      20.0,       1}
};

#define DESC_MAX_LEN 32
char flutterDescriptions[NUM_OPTIONS][DESC_MAX_LEN] = {
    "Off", 
    "Flat PolNo", 
    "Forest PolNo", 
    "Noon PolNo", 
    "Morning PolNo", 
    "Flat PolYes", 
    "Forest PolYes", 
    "Noon PolYes", 
    "Morning PolYes",
    "Forest PolNo 2Hz",
    "Forest PolNo 4Hz",
    "Forest PolNo 8Hz",
    "Forest PolNo Static"
};
