// TODO: Serial port timeout from Lucy
// TODO: Print complete bank
// TODO: Reorder pwmPins and ledWls to have LEDs in ascending order
// TODO: Solder channels 25 and 32
// TODO: Make light flicker at some F, think about the steepness of the transition


#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

#define NUM_CHANNELS 12
const int pwmPins[NUM_CHANNELS] = {  3,   4,   5,   6,   9,  10,  20,  21,  22,  23,  25,  32}; // PWM-capable pins
const int ledWls[NUM_CHANNELS]  = {100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111}; // WLS

int pwmValues[NUM_CHANNELS] = {0}; // Start with 0% duty cycle
int pwmFrequency = 500; // Default PWM frequency in Hz
int pwmResolution = 12; // Default bit resolution (user-definable)

const int NUM_SLOTS = 5;
int pwmBank[NUM_SLOTS][NUM_CHANNELS] = {
  {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0},
  {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023},
  {2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047},
  {3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071, 3071},
  {4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095}
};

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

void setupPWM() {
    Serial.begin(115200);
    showWelcomeScreen();
    setPWMResolution(pwmResolution);
}

void loopPWM() {
    processSerialInput();
}

void setup() {
    setupPWM();
}

void loop() {
    loopPWM();
}

#endif
