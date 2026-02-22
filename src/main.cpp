/*

  M5Stack Tab5 and TI INA3221 Current Sensor

  by Bryan A. "CrazyUncleBurton" Thompson

  Last Updated 2/19/2026

*/


#include <M5Unified.h>
#include <Wire.h>
#include "Adafruit_INA3221.h"


// Create an INA3221 object
Adafruit_INA3221 ina3221;

// Create an M5.Display object
M5GFX& display = M5.Display;

// Create an M5Canvas object
M5Canvas canvas(&display);


// Variables
int textSize = 2;
int lineHeight = 50;
int x_margin = 25; 
int y_header = 25;
int x = x_margin;
int y = y_header;
const int i2cSda = 53;
const int i2cScl = 54;


void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);

  Wire.begin(i2cSda, i2cScl);
  delay(1000); // Wait for I2C bus to stabilize

  // Set Display and Canvas Properties
  display.setRotation(1);
  canvas.setColorDepth(16);
  canvas.createSprite(display.width(), display.height());
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setFont(&fonts::efontJA_24_b);
  canvas.setTextSize(textSize);
  canvas.setTextWrap(false, false);

  // Draw Splash Canvas
  canvas.fillScreen(TFT_BLACK);
  y=200;
  x=475;
  canvas.setCursor(x, y);
  canvas.printf(" M5Stack Tab5");
  y=275;
  canvas.setCursor(x, y);
  canvas.printf("  Starting...");

  // Initialize the INA3221
  if (!ina3221.begin(0x40, &Wire)) { // can use other I2C addresses or buses
    y=425;
    canvas.setCursor(x, y);
    canvas.printf("Failed to find INA3221 chip");
    canvas.pushSprite(0, 0);
    while (1)
      delay(10);
  }
  y=425;
  canvas.setCursor(x, y);
  canvas.printf("INA3221 Found!");
  y += lineHeight;

  // Set INA3221 mode
  ina3221.setAveragingMode(INA3221_AVG_16_SAMPLES);

  // Set shunt resistances for all channels to 0.05 ohms
  for (uint8_t i = 0; i < 3; i++) {
    ina3221.setShuntResistance(i, 0.05);
  }

  // Make canvas visible on LCD
  canvas.pushSprite(0, 0);
  delay(3000);

}

void drawDashboard()
{

  y = y_header;
  x = x_margin;
  canvas.fillScreen(TFT_BLACK);
  canvas.setTextSize(textSize);

  //Display voltage and current (in mA) for each of the three channels
  for (uint8_t i = 0; i < 3; i++) {
    float voltage = ina3221.getBusVoltage(i);
    float current = ina3221.getCurrentAmps(i) * 1000; // Convert to mA
    canvas.setCursor(x, y);
    canvas.printf("Channel %d Voltage = %.2fV Current = %.2fmA", i, voltage, current);
    y += lineHeight;
  }

  canvas.pushSprite(0, 0);
  y=y_header; // Reset y for next update
}

void loop()
{
  // Only update screen every minute
  static uint32_t lastFrameMs = 0;
  const uint32_t frameIntervalMs = 1000;

  if (millis() - lastFrameMs < frameIntervalMs) {
    return;
  }
  lastFrameMs = millis();

  drawDashboard();

}
