const int BTN_UP_PIN = 1;  // Button to increase the selected flutter option
const int BTN_DW_PIN = 0;  // Button to decrease the selected flutter option

const int TTL_IN_PIN  = 2;
const int TTL_OUT_PIN = 3;

#define NUM_CHANNELS 16
//                                   1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16
const int pwmPins[NUM_CHANNELS] = {  7,  20,  14,   9,  11,  16,  22,  10,  18,  13,  12,  17,  19,  15,   8,  21}; // PWM-capable pins
const int ledWls[NUM_CHANNELS]  = {364, 382, 390, 402, 424, 427, 444, 466, 488, 514, 549, 594, 620, 627, 656, 679}; // WLS

const int NUM_SLOTS = 17; 
int pwmBank[NUM_SLOTS][NUM_CHANNELS] = {
//   364,   382,   390,   402,   424,   427,   444,   466,   488,   514,   549,   594,   620,   627,   656,   679
  {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  {65535,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  {    0, 65535,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  {    0,     0, 65535,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  {    0,     0,     0, 65535,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  {    0,     0,     0,     0, 65535,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  {    0,     0,     0,     0,     0, 65535,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  {    0,     0,     0,     0,     0,     0, 65535,     0,     0,     0,     0,     0,     0,     0,     0,     0},
  {    0,     0,     0,     0,     0,     0,     0, 65535,     0,     0,     0,     0,     0,     0,     0,     0},
  {    0,     0,     0,     0,     0,     0,     0,     0, 65535,     0,     0,     0,     0,     0,     0,     0},
  {    0,     0,     0,     0,     0,     0,     0,     0,     0, 65535,     0,     0,     0,     0,     0,     0},
  {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 65535,     0,     0,     0,     0,     0},
  {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 65535,     0,     0,     0,     0},
  {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 65535,     0,     0,     0},
  {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 65535,     0,     0},
  {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 65535,     0},
  {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 65535}
};

int flashPWMValue[NUM_CHANNELS] = {65535, 57472, 27129, 21156, 22061, 15747, 17545, 19292, 27213, 29319, 22277, 36909, 26312, 15503, 14344, 13067};
int flashDuration_ms = 150; // Duration for which the light stays on during a flash
int TTLdebouceTime_ms = 15; // Minimum time between TTL triggers to avoid multiple triggers from a single pulse
bool serialTTLordered = false;

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
    "|----------------|",
    "|x---------------|",
    "|-x--------------|",
    "|--x-------------|",
    "|---x------------|",
    "|----x-----------|",
    "|-----x----------|",
    "|------x---------|",
    "|-------x--------|",
    "|--------x-------|",
    "|---------x------|",
    "|----------x-----|",
    "|-----------x----|",
    "|------------x---|",
    "|-------------x--|",
    "|--------------x-|",
    "|---------------x|"
};
