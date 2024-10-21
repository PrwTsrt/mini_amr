#include <Wire.h>
#include "Adafruit_VL6180X.h"
#include <ACROBOTIC_SSD1306.h>

Adafruit_VL6180X vl = Adafruit_VL6180X();

void setup() {
  // Serial.begin(115200);
  Wire.begin();
  vl.begin();
  oled.init();                      // Initialze SSD1306 OLED display
  oled.clearDisplay();              // Clear screen
  oled.setTextXY(3,1);   
  oled.putString("NECTEC SMR 410");

  delay(2000);

  // wait for serial port to open on native usb devices
  // while (!Serial) {
  //   delay(1);
  // }
  
  // Serial.println("Adafruit VL6180x test!");
  // if (! vl.begin()) {
  //   Serial.println("Failed to find sensor");
  //   while (1);
  // }
  // Serial.println("Sensor found!");
}

void loop() {
//   float lux = vl.readLux(VL6180X_ALS_GAIN_5);

//   Serial.print("Lux: "); Serial.println(lux);
  
  // uint8_t range = vl.readRange();
  // uint8_t status = vl.readRangeStatus();

  // if (status == VL6180X_ERROR_NONE) {
    // Serial.print("Range: "); Serial.println(range);
  // }
  int analogValue = analogRead(28);

  // String rang2print = String(range);
  String rang2print = String(analogValue);

  // oled.clearDisplay();              
  oled.setTextXY(2,1);   
  oled.putString(rang2print);

  // // Some error occurred, print it out!
  
  // if  ((status >= VL6180X_ERROR_SYSERR_1) && (status <= VL6180X_ERROR_SYSERR_5)) {
  //   Serial.println("System error");
  // }
  // else if (status == VL6180X_ERROR_ECEFAIL) {
  //   Serial.println("ECE failure");
  // }
  // else if (status == VL6180X_ERROR_NOCONVERGE) {
  //   Serial.println("No convergence");
  // }
  // else if (status == VL6180X_ERROR_RANGEIGNORE) {
  //   Serial.println("Ignoring range");
  // }
  // else if (status == VL6180X_ERROR_SNR) {
  //   Serial.println("Signal/Noise error");
  // }
  // else if (status == VL6180X_ERROR_RAWUFLOW) {
  //   Serial.println("Raw reading underflow");
  // }
  // else if (status == VL6180X_ERROR_RAWOFLOW) {
  //   Serial.println("Raw reading overflow");
  // }
  // else if (status == VL6180X_ERROR_RANGEUFLOW) {
  //   Serial.println("Range reading underflow");
  // }
  // else if (status == VL6180X_ERROR_RANGEOFLOW) {
  //   Serial.println("Range reading overflow");
  // }
  delay(1000);
}