const int BTN_UP_PIN = 1;  // Button to increase the selected flutter option
const int BTN_DW_PIN = 0;  // Button to decrease the selected flutter option

#define NUM_CHANNELS 16
//                                   1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16
const int pwmPins[NUM_CHANNELS] = {  7,  20,  14,   9,  11,  16,  22,  10,  18,  13,  12,  17,  19,  15,   8,  21}; // PWM-capable pins
const int ledWls[NUM_CHANNELS]  = {364, 382, 390, 402, 424, 427, 444, 466, 488, 514, 549, 594, 620, 627, 656, 679}; // WLS

const int NUM_SLOTS = 17; 
int pwmBank[NUM_SLOTS][NUM_CHANNELS] = {
//  364,  382,  390,  402,  424,  427,  444,  466,  488,  514,  549,  594,  620,  627,  656,  679
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {4095,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0, 4095,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0, 4095,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0, 4095,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0, 4095,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0, 4095,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0, 4095,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0, 4095,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0, 4095,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0, 4095,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 4095,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 4095,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 4095,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 4095,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 4095,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 4095}
};

// flutterParams holds the control values for switching between pwmBanks with a particular transition
// Position 0: light setting 1
// Position 1: light setting 2
// Position 2: frequency [Hz]
// Position 3: duty cycle
// Position 4: part of duty cycle for transitions
// Position 5: type of transition (0: linear, 1: sine)
const int NUM_OPTIONS = 17;
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
    { 0,     9,      1.0,     75.0,      20.0,       1},
    { 0,    10,      1.0,     75.0,      20.0,       1},
    { 0,    11,      1.0,     75.0,      20.0,       1},
    { 0,    12,      1.0,     75.0,      20.0,       1},
    { 0,    13,      1.0,     75.0,      20.0,       1},
    { 0,    14,      1.0,     75.0,      20.0,       1},
    { 0,    15,      1.0,     75.0,      20.0,       1},
    { 0,    16,      1.0,     75.0,      20.0,       1}
};

#define DESC_MAX_LEN 32
char flutterDescriptions[NUM_OPTIONS][DESC_MAX_LEN] = {
    "Off", 
    "One light slow", 
    "Two lights slow", 
    "All lights fade", 
    "Pattern A", 
    "Pattern B", 
    "Pattern C", 
    "Pattern D", 
    "Pulse mode", 
    "Alt flash",
    "Epileptic flash"
};
