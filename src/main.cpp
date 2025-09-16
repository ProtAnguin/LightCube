#ifndef MAIN_H
#define MAIN_H

#include <hardware/clocks.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>
#include "easterFile.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1           // Most I2C modules don’t expose reset; use -1
#define OLED_ADDR    0x3C         // Common SSD1306 I2C address (sometimes 0x3D)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned DISPLAY_IDLE_MS = 5000;   // <- adjust as you like
static unsigned long lastActivityMs = 0;
static bool displayDimmed = false;

inline void markActivity() {
  lastActivityMs = millis();
}

#define BTN_DEBOUNCE_INTERVAL_MS 50 // Debounce interval for buttons in milliseconds

Bounce btnU;
Bounce btnD;

// --- Secret combo (simple) ---
const uint8_t  SECRET_TARGET           = 4;     // number of U+D catches
const unsigned long SECRET_HOLD_MS     = 300;   // hold U at least this long
const unsigned long SECRET_TIMEOUT_MS  = 1500;  // after arming, how long to wait for D
const unsigned long CLICK_MAX_MS       = 250;   // short press window

// Secret progress counter
static uint8_t secretCount = 0;

// U click/hold tracking
static bool uDown = false;
static unsigned long uDownMs = 0;

// "Armed" means: U has been held long enough; waiting for D
static bool uArmed = false;
static unsigned long uArmedSince = 0;

// D click tracking and one-shot ignore after combo
static bool dDown = false;
static unsigned long dDownMs = 0;
static bool dIgnoreReleaseOnce = false;

inline void secretMarkLeft()  { display.drawPixel(0,   0, SSD1306_WHITE); display.display(); }
inline void secretMarkRight() { display.drawPixel(127, 0, SSD1306_WHITE); display.display(); }
inline void secretClearMarks(){ display.drawPixel(0,0,SSD1306_BLACK); display.drawPixel(127,0,SSD1306_BLACK); display.display(); }

// helpers
static inline int wrap_inc(int v, int n) {        // (v+1) % n
  v++; if (v >= n) v = 0; return v;
}
static inline int wrap_dec(int v, int n) {        // (v-1+n) % n without negatives
  v--; if (v < 0) v = n - 1; return v;
}

// SELECT DEVICE ====== SELECT DEVICE ====== SELECT DEVICE ====== SELECT DEVICE ====== SELECT DEVICE ====== SELECT DEVICE ====== SELECT DEVICE
#include "LightCube2.h"
//#include "LightCube3.h"

int pwmValues[NUM_CHANNELS] = {0}; // Start with 0% duty cycle
int pwmFrequency = 500; // Default PWM frequency in Hz
int pwmResolution = 16; // Default bit resolution (user-definable)

#define MIN_FREQ_SETTING 0.01    // Minimum frequency setting
#define MAX_FREQ_SETTING 100.0  // Maximum frequency setting

bool switchesInUse = false; // Flag to indicate if switches are in use

int selectedFlutter = 0; // Default selected flutter option
unsigned long timePoints[5] = {0, 1, 500000, 500001, 999999}; // Time points for the five phases of the cycle (last value is end of cycle)
unsigned long cycleTimeCurent = 0; // Current time in the cycle
unsigned long cycleTimeStart = 0; // Last time the lights were updated

void showWelcomeScreen() {
    Serial.println("\n========================================");
    Serial.println(" PWM Controller for Raspberry Pi Pico 2");
    Serial.println("========================================");
    Serial.println(" Type 'H*' for help.");
    Serial.println("========================================\n");
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
  uint32_t sysHz = clock_get_hz(clk_sys);
  // Keep your existing relationship between clock and PWM freq:
  // freq = clock / 2^resolution
  pwmFrequency = sysHz / (1u << pwmResolution);

  // Arduino-Pico: set freq once, globally; then set the duty "range"/resolution
  analogWriteFreq(pwmFrequency);                   // global PWM frequency
  analogWriteRange((1u << pwmResolution) - 1u);    // duty steps = 2^resolution - 1

  for (int i = 0; i < NUM_CHANNELS; i++) {
    pinMode(pwmPins[i], OUTPUT);
    analogWrite(pwmPins[i], 0); // Reset duty to 0% in the new range
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

void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(flutterDescriptions[selectedFlutter]);
  
  display.setTextSize(7);
  char bufProg[4];  // enough for "99\0"
  sprintf(bufProg, "%2d", selectedFlutter);  // right-align in 2 spaces
  display.println(bufProg);

  display.setTextSize(1);
  uint8_t Hofs = 86;
  uint8_t Vofs = 9;
  uint8_t curLine = 1;
  char buf[32];

  display.setCursor(Hofs, curLine++*Vofs);
  sprintf(buf, "A: %4d", (int)flutterParams[selectedFlutter][0]);
  display.print(buf);

  display.setCursor(Hofs, curLine++*Vofs);
  sprintf(buf, "B: %4d", (int)flutterParams[selectedFlutter][1]);
  display.print(buf);

  display.setCursor(Hofs, curLine++*Vofs);
  sprintf(buf, "F: %4.1f", (float)flutterParams[selectedFlutter][2]);
  display.print(buf);

  display.setCursor(Hofs, curLine++*Vofs);
  sprintf(buf, "D: %4.1f", (float)flutterParams[selectedFlutter][3]);
  display.print(buf);

  display.setCursor(Hofs, curLine++*Vofs);
  sprintf(buf, "P: %4.1f", (float)flutterParams[selectedFlutter][4]);
  display.print(buf);

  display.setCursor(Hofs, curLine++*Vofs);
  sprintf(buf, "M: %4d", (int)flutterParams[selectedFlutter][5]);
  display.print(buf);
  
  display.display(); // Push buffer to screen
}

void wakeDisplay() {
  if (displayDimmed) {
    display.dim(false);     // undim
    displayDimmed = false;
    updateDisplay();        // redraw at normal brightness
  }
  markActivity();
}

void maybeDimDisplay() {
  if (!displayDimmed && (millis() - lastActivityMs >= DISPLAY_IDLE_MS)) {
    display.dim(true);      // lower brightness (keeps pixels; gentle on OLED)
    displayDimmed = true;
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

    updateDisplay();
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
    markActivity();
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
  btnU.update();
  btnD.update();

  if (btnU.changed() || btnD.changed()) markActivity();

  // Wake-on-first-press when dimmed
  if (displayDimmed && (btnU.fell() || btnD.fell())) { wakeDisplay(); return; }

  // -------- U press / release --------
  if (btnU.fell()) {
    uDown = true;
    uDownMs = millis();
  }

  // Arm the secret once U has been held long enough
  if (uDown && !uArmed && (millis() - uDownMs >= SECRET_HOLD_MS)) {
    uArmed = true;
    uArmedSince = millis();
    secretMarkLeft();   // show left pixel
  }

  // If armed too long without D, time out and clear
  if (uArmed && (millis() - uArmedSince > SECRET_TIMEOUT_MS)) {
    uArmed = false;
    secretClearMarks();
  }

  if (btnU.rose()) {
    // If not armed, treat as a normal short click (on release)
    if (!uArmed && (millis() - uDownMs) <= CLICK_MAX_MS) {
      selectedFlutter = wrap_inc(selectedFlutter, NUM_OPTIONS);
      updateFlutterTimePoints();
      reportFlutterParameters();
    }
    // If armed but never completed with D, just cancel the arm
    if (uArmed) {
      uArmed = false;
      secretClearMarks();
    }
    uDown = false;
  }

  // -------- D press / release --------
  if (btnD.fell()) {
    // If U is armed and still held: this is a combo catch
    if (uArmed && uDown) {
      secretMarkRight();
      secretCount++;
      delay(100);
      secretClearMarks();

      // Re-arm next round
      uArmed = false;
      // Ignore this release as a normal click
      dIgnoreReleaseOnce = true;

      if (secretCount >= SECRET_TARGET) {
        draw_easterIm1(display);
        secretCount = 0;             // allow repeating
      }
    } else {
      // Normal D path: start timing for short/long classification
      dDown = true;
      dDownMs = millis();
    }
  }

  if (btnD.rose()) {
    if (dIgnoreReleaseOnce) {
      dIgnoreReleaseOnce = false;    // consume the release after combo
    } else if (dDown) {
      unsigned long dt = millis() - dDownMs;
      if (dt <= CLICK_MAX_MS) {
        // Normal short click (on release)
        selectedFlutter = wrap_dec(selectedFlutter, NUM_OPTIONS);
        updateFlutterTimePoints();
        reportFlutterParameters();
      }
    }
    dDown = false;
  }
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

void setupDisplay() {
    Wire.begin();

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("SSD1306 init failed. Check wiring/address.");
        for (;;);
    }

    display.clearDisplay();
    
    updateDisplay(); // Initial display update
    markActivity();
}

void setupButtons() {
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DW_PIN, INPUT_PULLUP);

  btnU.attach(BTN_UP_PIN);   // no mode here; we already set it
  btnD.attach(BTN_DW_PIN);

  btnU.interval(BTN_DEBOUNCE_INTERVAL_MS);
  btnD.interval(BTN_DEBOUNCE_INTERVAL_MS);
}

void setupPWM() {
    Serial.begin(115200);
    showWelcomeScreen();
    setPWMResolution(pwmResolution);
    limitFlutterParameters();
}

void loopPWM() {
    processSerialInput();
    checkButtonAndCycleFlutter(); // Check button state and cycle through flutter options
    
    if(selectedFlutter > 0) { // If flutter is active
        updateLights();
    }

    maybeDimDisplay();
}

void setup() {
    setupPWM();
    setupButtons();
    setupDisplay();
}

void loop() {
    loopPWM();
}

#endif
