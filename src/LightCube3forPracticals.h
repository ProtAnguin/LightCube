const int BTN_UP_PIN = 1;  // Button to increase the selected flutter option
const int BTN_DW_PIN = 0;  // Button to decrease the selected flutter option

const int TTL_IN_PIN  = 2;
const int TTL_OUT_PIN = 3;

constexpr uint8_t TTL_TRIGGER_ACTIVE_STATE = LOW;   // set to LOW for active-low trigger

#define NUM_CHANNELS 16
//                                   1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16
const int pwmPins[NUM_CHANNELS] = {  7,  20,  14,   9,  11,  16,  22,  10,  18,  13,  12,  17,  19,  15,   8,  21}; // PWM-capable pins
const int ledWls[NUM_CHANNELS]  = {363, 371, 389, 401, 422, 427, 444, 465, 486,   0, 547, 593, 621, 626, 656, 679}; // WLS

const int NUM_SLOTS = 17; 
int pwmBank[NUM_SLOTS][NUM_CHANNELS] = {
//   363,   371,   389,   401,   422,   427,   444,   465,   486,     0,   547,   593,   621,   626,   656,   679
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

int flashPWMValue[NUM_CHANNELS] = {65535, 56422, 30510, 21874, 24228, 16781, 17863, 20852, 27455,     0, 23522, 39233, 28826, 18144, 15216, 14075}; // PWM values for each channel during a flash, calculated based on the desired flash intensity and the LED characteristics

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
