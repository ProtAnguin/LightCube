// TODO: Serial port timeout from Lucy
// TODO: Print complete bank
// TODO: Reorder pwmPins and ledWls to have LEDs in ascending order
// TODO: Solder channels 25 and 32
// TODO: Make light flicker at some F, think about the steepness of the transition
// TODO: timePoints have to be at least increasing by 1 to avoid division by zero
// TODO: The 8 switches should connect to GND as they are INPUT_PULLUP
// TODO: Add a set command for adjusting the PWM of each channels for calib with MATLAB setXXYYYY* to set the value of channel XX to YYYY

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

#define NUM_CHANNELS 12
const int pwmPins[NUM_CHANNELS] = {  4,   5,   9,  25,   6,  20,  22,  21,  23,  10,   3,  32}; // PWM-capable pins
const int ledWls[NUM_CHANNELS]  = {363, 369, 393, 400, 422, 435, 453, 471, 491, 514, 540, 632}; // WLS

int pwmValues[NUM_CHANNELS] = {0}; // Start with 0% duty cycle
int pwmFrequency = 500; // Default PWM frequency in Hz
int pwmResolution = 12; // Default bit resolution (user-definable)

const int NUM_SLOTS = 9;
int pwmBank[NUM_SLOTS][NUM_CHANNELS] = {
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023},
  {2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047},
  {3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071},
  {4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095},
  {4095,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 4095},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 4095},
  {   0,    0,    0,    0,    0,    0,    0, 4095,    0, 4095,    0,    0}
};

// flutterParams holds the control values for switching between pwmBanks with a particular transition
// Position 0: light setting 1
// Position 1: light setting 2
// Position 2: frequency
// Position 3: duty cycle
// Position 4: part of duty cycle for transitions
// Position 5: type of transition (0: linear, 1: sine)
const int NUM_OPTIONS = 14;
float flutterParams[NUM_OPTIONS][6] = {
    {0,     0,      1.0,     20.0,       0.0,       0},
    {0,     5,      1.0,     20.0,       0.0,       0},
    {0,     5,      1.0,     40.0,      20.0,       0},
    {0,     5,      1.0,     60.0,      20.0,       1},
    {0,     5,      0.4,     50.0,      50.0,       1},
    {0,     8,      0.4,     50.0,      50.0,       1},
    {5,     8,      0.4,     50.0,      50.0,       1},
    {5,     7,      0.4,     50.0,      50.0,       1},
    {0,     5,      1.0,     30.0,      30.0,       1},
    {0,     5,      2.0,     30.0,      30.0,       1},
    {0,     5,      4.0,     30.0,      30.0,       1},
    {1,     5,      1.0,     50.0,      25.0,       0},
    {0,     5,      1.0,    100.0,       0.0,       1},
    {0,     5,      1.0,    100.0,      20.0,       1}
};

#define MIN_FREQ_SETTING 0.01    // Minimum frequency setting
#define MAX_FREQ_SETTING 100.0  // Maximum frequency setting

const int buttonPin = 7; // Pin for the button

#define NUM_SWITCHES 6 // Define the number of switches being used (can be adjusted)
const int switchPins[8] = {0, 1, 2, 8, 11, 12, 15, 14}; // Pins for the 8 switches
bool switchesInUse = false; // Flag to indicate if switches are in use

int selectedFlutter = 0; // Default selected flutter option
unsigned long timePoints[5] = {0, 1, 500000, 500001, 999999}; // Time points for the five phases of the cycle (last value is end of cycle)
unsigned long cycleTimeCurent = 0; // Current time in the cycle
unsigned long cycleTimeStart = 0; // Last time the lights were updated

void showWelcomeScreen() {
    Serial.println("\n================================");
    Serial.println(" PWM Controller for Teensy 3.2 ");
    Serial.println("================================");
    Serial.println(" Type 'H*' for help.");
    Serial.println("================================\n");
}

void showHelpScreen() {
    Serial.println("\n========= HELP MENU =========");
    Serial.println("Available Commands:");
    Serial.println(" C<channel>,<value>* - Set PWM for a channel");
    Serial.println(" R<bitdepth>* - Set PWM resolution (8-16 bits)");
    Serial.println(" W* - Show welcome screen");
    Serial.println(" H* - Show this help menu");
    Serial.println(" S* - Show current PWM status");
    Serial.println(" B<bank>* - Load a bank of values");
    Serial.println(" F<flutter>* - Set flutter parameters");
    Serial.println("----------------------------");
    Serial.println("Channels used: 3, 4, 5, 6, 9, 10, 20, 21, 22, 23, 25, 32");
    Serial.println("Bit depth affects duty cycle range (e.g., 8-bit: 0-255, 12-bit: 0-4095)");
    Serial.println("Changing bit depth resets all duty cycles to 0");
    Serial.println("============================\n");
}

void showStatus() {
    int maxPWMValue = (1 << pwmResolution) - 1; // Maximum PWM value based on resolution
    Serial.println("\n========= PWM STATUS =========");
    Serial.printf("CPU Speed: %lu Hz\n", F_CPU);
    Serial.printf("Bus Speed: %lu Hz\n", F_BUS);
    Serial.printf("Bit Depth: %d-bit\n", pwmResolution);
    Serial.printf("Current PWM Frequency: %d Hz\n", pwmFrequency);
    Serial.printf("Maximum Possible PWM Value: %d\n", maxPWMValue); // Report the maximum PWM value
    Serial.println("Channel | PWM value | peak WL");
    Serial.println("----------------------------");
    for (int i = 0; i < NUM_CHANNELS; i++) {
        Serial.printf("   %2d   |   %4d    | %3d \n", pwmPins[i], pwmValues[i], ledWls[i]);
    }
    Serial.println("============================\n");
}

void updatePWMSettings() {
    uint32_t busSpeed = F_BUS; // Get F_BUS clock speed
    pwmFrequency = busSpeed / (1 << pwmResolution); // Adjust calculation using F_BUS
    for (int i = 0; i < NUM_CHANNELS; i++) {
        analogWriteFrequency(pwmPins[i], pwmFrequency);
        analogWrite(pwmPins[i], 0); // Reset duty cycle to 0%
    }
}

void setPWMResolution(int resolution) {
    if (resolution >= 8 && resolution <= 16) { // Allow 8 to 16-bit resolution
        pwmResolution = resolution;
        analogWriteResolution(pwmResolution);
        
        // Reset all duty cycles to 0
        for (int i = 0; i < NUM_CHANNELS; i++) {
            pwmValues[i] = 0;
            analogWrite(pwmPins[i], 0); // Set all PWM outputs to 0
        }
        
        updatePWMSettings();
    }
}

void setPWMDutyCycle(int channel, int value) {
    int maxValue = (1 << pwmResolution) - 1;
    if (channel >= 0 && channel < NUM_CHANNELS && value >= 0 && value <= maxValue) {
        pwmValues[channel] = value;
        analogWrite(pwmPins[channel], pwmValues[channel]);
    }
}

void limitFlutterParameters() {
    // TODO: Implement a function to limit the flutter parameters
    // This function should ensure that the values in flutterParams are within acceptable ranges
    for (int i = 0; i < NUM_OPTIONS; i++) {
        flutterParams[i][0] = constrain(flutterParams[i][0], 0, NUM_SLOTS - 1); // Light setting 1
        flutterParams[i][1] = constrain(flutterParams[i][1], 0, NUM_SLOTS - 1); // Light setting 2
        flutterParams[i][2] = constrain(flutterParams[i][2], MIN_FREQ_SETTING, MAX_FREQ_SETTING); // Frequency
        flutterParams[i][3] = constrain(flutterParams[i][3], 0, 100); // Duty cycle
        if (flutterParams[i][3] < 50) {
            flutterParams[i][4] = constrain(flutterParams[i][4], 0, flutterParams[i][3]); // Transition part of duty cycle
        } else {
            flutterParams[i][4] = constrain(flutterParams[i][4], 0, 100 - flutterParams[i][3]); // Transition part of duty cycle
        }
        flutterParams[i][5] = constrain(flutterParams[i][5], 0, 1); // Transition type (0: linear, 1: sine)
    }
}

void updateFlutterTimePoints() {
    // Turn off all LEDs before updating
    for (int i = 0; i < NUM_CHANNELS; i++) {
        setPWMDutyCycle(i, pwmBank[0][i]);
    }

    timePoints[0] = 0;                                                                          // Start of the cycle (maybe usefull later in development to change)
    timePoints[4] = (1000000 / flutterParams[selectedFlutter][2])-1;                            // Length of the cycle (1e6 / frequency)
    timePoints[2] = timePoints[4] * flutterParams[selectedFlutter][3] / 100.0;                  // End of ON phase
    timePoints[1] = timePoints[0] + timePoints[2] * flutterParams[selectedFlutter][4] / 100.0;  // End of the FIRST  transition
    timePoints[3] = timePoints[2] + timePoints[2] * flutterParams[selectedFlutter][4] / 100.0;  // End of the SECOND transition
    
    // Ensure time points are in increasing order
    for (int i = 0; i < 4; i++) {
        if (timePoints[i] >= timePoints[i + 1]) {
            timePoints[i + 1] = timePoints[i] + 1; // Ensure increasing order
        }
    }
}

void reportFlutterParameters() {
    Serial.println("-Flutter--------------------------------------");
    Serial.printf("  Selected flutter: %d\n", selectedFlutter);
    Serial.printf("           Light 1: %d\n", (int)flutterParams[selectedFlutter][0]);
    Serial.printf("           Light 2: %d\n", (int)flutterParams[selectedFlutter][1]);
    Serial.printf("         Frequency: %3.1f\n", flutterParams[selectedFlutter][2]);
    Serial.printf("        Duty cycle: %3.1f\n", flutterParams[selectedFlutter][3]);
    Serial.printf("   Transition part: %3.1f\n", flutterParams[selectedFlutter][4]);
    Serial.printf("   Transition mode: %d\n", flutterParams[selectedFlutter][5]);
    Serial.printf("  Time points [us]: %lu, %lu, %lu, %lu, %lu\n", timePoints[0], timePoints[1], timePoints[2], timePoints[3], timePoints[4]);
}

void parseSerialCommand(String command) {
    if (command.startsWith("C")) { // Set PWM: C<channel>,<value>*
        int commaIndex = command.indexOf(',');
        if (commaIndex > 0) {
            int channel = command.substring(1, commaIndex).toInt();
            int value = command.substring(commaIndex + 1).toInt();
            setPWMDutyCycle(channel, value);
        }
    } else if (command.startsWith("R")) { // Set resolution: R<bitdepth>*
        int resolution = command.substring(1).toInt();
        setPWMResolution(resolution);
    } else if (command.startsWith("W")) { // Show welcome screen: W*
        showWelcomeScreen();
    } else if (command.startsWith("H")) { // Show help screen: H*
        showHelpScreen();
    } else if (command.startsWith("S")) { // Show status: S*
        showStatus();
    } else if (command.startsWith("B")) { // Load bank: B<value>*
        int tempBankLine = constrain(command.substring(1).toInt(), 0, NUM_SLOTS-1);
        Serial.printf(" Bank selected: %d\n", tempBankLine);
        for (int i = 0; i < NUM_CHANNELS; i++) {
            setPWMDutyCycle(i, pwmBank[tempBankLine][i]);
        }
        showStatus();
    } else if (command.startsWith("F")) { // Flutter part
        selectedFlutter = constrain(command.substring(1).toInt(), 0, NUM_OPTIONS-1);
        updateFlutterTimePoints();
        reportFlutterParameters();
    } else if (command.startsWith("set")) { // Set PWM for a specific channel: setXXYYYY*
        int channel = command.substring(3, 5).toInt(); // Extract channel number (XX)
        int value = command.substring(5).toInt();   // Extract PWM value (YYYY)
        setPWMDutyCycle(channel, value);              // Set the PWM value for the channel
    }
 }

void processSerialInput() {
    static String inputString = "";
    while (Serial.available()) {
        char inChar = Serial.read();
        if (inChar == '*') {
            parseSerialCommand(inputString);
            inputString = "";
        } else {
            inputString += inChar;
        }
    }
}

void updateLights() {
    unsigned long transitionTime = 0;
    float transitionProgress = 0;
    float transitionValue = 0;
    // Continue here with the logic for updating the lights based on the time points and flutter parameters
    cycleTimeCurent = micros() - cycleTimeStart;

    if( cycleTimeCurent > timePoints[4] ) { // End of cycle reacher, reset the cycle
        cycleTimeStart = micros();
        cycleTimeCurent = 0;
    }

    if (cycleTimeCurent < timePoints[0]) { // Pre-cycle time
        transitionProgress = 0;
    } else if (cycleTimeCurent < timePoints[1]) { // First transition
        transitionTime = timePoints[1] - timePoints[0];
        transitionProgress = (float)(cycleTimeCurent - timePoints[0]) / transitionTime;
    } else if (cycleTimeCurent < timePoints[2]) {
        transitionProgress = 1;
    } else if (cycleTimeCurent < timePoints[3]) {
        transitionTime = timePoints[3] - timePoints[2];
        transitionProgress = 1.0 - ((float)(cycleTimeCurent - timePoints[2]) / transitionTime);
    } else {
        transitionProgress = 0;
    }

    // Once we have the transitionProgress value calculate PWM and set it
    if(flutterParams[selectedFlutter][5] == 0) { // Linear transition
        transitionValue = transitionProgress;
    } else if (flutterParams[selectedFlutter][5] == 1) { // Sine transition
        transitionValue = 0.5 * (1 - cos(transitionProgress * PI)); // Sine transition
    }
    for (int i = 0; i < NUM_CHANNELS; i++) {
        int val1 = pwmBank[(int)flutterParams[selectedFlutter][0]][i];
        int val2 = pwmBank[(int)flutterParams[selectedFlutter][1]][i];
        setPWMDutyCycle(i, int(val1 + (val2 - val1) * transitionValue));
    }
}

void checkButtonAndCycleFlutter() {

    if (digitalRead(buttonPin) == HIGH) { // Button is pressed and was not already detected
        selectedFlutter = (selectedFlutter + 1) % NUM_OPTIONS;  // Increment selectedFlutter and loop back to 0 if it exceeds the max value
        updateFlutterTimePoints();                              // Update the flutter time points for the new selection
        reportFlutterParameters();
        while( digitalRead(buttonPin) == HIGH) { delay(10); }   // Wait for button release;
    }
}

int readSwitches() {
    int value = 0; // Initialize the encoded value to 0

    for (int i = 0; i < NUM_SWITCHES; i++) {
        if (digitalRead(switchPins[i]) == LOW) { // Check if the switch is pressed (LOW due to INPUT_PULLUP)
            value |= (1 << i); // Set the corresponding bit in the value
        }
    }

    return value; // Return the encoded value (0–255)
}

String getBinaryValue(int value, int numBits, char offChar = '_', char onChar = 'X') {
    String binaryString = ""; // Initialize an empty string to hold the binary representation

    for (int i = numBits - 1; i >= 0; i--) { // Iterate from the most significant bit to the least significant bit
        if (value & (1 << i)) { // Check if the i-th bit is set
            binaryString += onChar; // Append the character for bit value 1
        } else {
            binaryString += offChar; // Append the character for bit value 0
        }
    }

    return binaryString; // Return the binary representation as a string
}

void setupPWM() {
    Serial.begin(115200);
    showWelcomeScreen();
    setPWMResolution(pwmResolution);
    limitFlutterParameters();

    pinMode(buttonPin, INPUT); // Set button pin as input

    // Configure switch pins as inputs with pull-up resistors
    for (int i = 0; i < NUM_SWITCHES; i++) {
        pinMode(switchPins[i], INPUT_PULLUP);
    }
}

void loopPWM() {
    processSerialInput();
    // checkButtonAndCycleFlutter(); // Check button state and cycle through flutter options

    int switchValue = readSwitches(); // Get the encoded value from the switches
    if (switchValue != 0) { // If no switches are pressed --> skip
        switchesInUse = true;
        if (switchValue != selectedFlutter) { // Update only if the value has changed
            selectedFlutter = switchValue; // Set selectedFlutter to the encoded value
            updateFlutterTimePoints(); // Update the flutter time points
            Serial.printf("Switches encoded value: %3d %s\n", selectedFlutter, getBinaryValue(selectedFlutter, NUM_SWITCHES).c_str());
        }
    } else {
        if (switchesInUse) { // If switches were in use and now are not
            switchesInUse = false; // Reset the flag
            selectedFlutter = 0; // Reset selectedFlutter if no switches are pressed
            updateFlutterTimePoints();
            Serial.printf("Switches encoded value: %3d %s\n", selectedFlutter, getBinaryValue(selectedFlutter, NUM_SWITCHES).c_str());
        }
    }
    
    if(selectedFlutter > 0) { // If flutter is active
        updateLights();
    }
}

void setup() {
    setupPWM();
}

void loop() {
    loopPWM();
}

#endif
