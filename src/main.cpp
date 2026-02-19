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
const uint8_t inaAddress = 0x40;

M5GFX& display = M5.Display;
M5Canvas canvas(&display);


// Variables
int textSize = 4;
int lineHeight = 14;
int x_margin = 25; 
int y_header = 25;
const int i2cSda = 53;
const int i2cScl = 54;
const char degreeSymbol[] = { static_cast<char>(0xC2), static_cast<char>(0xB0), '\0' };

bool i2cAck(TwoWire& wire, uint8_t address)
{
  wire.beginTransmission(address);
  return wire.endTransmission() == 0;
}

bool readRegister16(TwoWire& wire, uint8_t address, uint8_t reg, uint16_t& value)
{
  wire.beginTransmission(address);
  wire.write(reg);
  if (wire.endTransmission(false) != 0) {
    return false;
  }

  if (wire.requestFrom(static_cast<int>(address), 2) != 2) {
    return false;
  }

  uint8_t msb = wire.read();
  uint8_t lsb = wire.read();
  value = (static_cast<uint16_t>(msb) << 8) | lsb;
  return true;
}

int findFirstI2CAddress(TwoWire& wire)
{
  for (uint8_t address = 1; address < 127; ++address) {
    if (i2cAck(wire, address)) {
      return address;
    }
  }
  return -1;
}

void drawDashboard()
{
  int x = x_margin;
  int y = y_header;

  canvas.fillScreen(TFT_BLACK);
  canvas.setTextSize(textSize);
  canvas.setCursor(x, y);

  // Display voltage and current (in mA) for each of the three channels
  for (uint8_t i = 0; i < 3; i++) {
    float voltage = ina3221.getBusVoltage(i);
    float current = ina3221.getCurrentAmps(i) * 1000; // Convert to mA
    canvas.setCursor(x, y);
    canvas.printf("Ch%d: Voltage=%.2fV Current=%.2fmA", i, voltage, current);
    y += lineHeight;
  }

  canvas.pushSprite(0, 0);
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);

  // Standard Arduino I2C on Tab5 Port A (SDA=53, SCL=54)
  Wire1.begin(i2cSda, i2cScl, 100000);
  delay(20);

  // Set Display and Canvas Properties
  display.setRotation(1);
  canvas.setColorDepth(16);
  canvas.createSprite(display.width(), display.height());
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setFont(&fonts::efontJA_24_b);
  canvas.setTextSize(textSize);
  canvas.setTextWrap(false, false);
  lineHeight = canvas.fontHeight() + 14;

  // Draw Splash Canvas
  int x = x_margin;
  int y = y_header;
  canvas.fillScreen(TFT_BLACK);
  canvas.setCursor(x, y);
  canvas.printf("M5Stack Tab5");
  y += lineHeight;
  canvas.setCursor(x, y);
  canvas.printf("Starting...");
  canvas.pushSprite(0, 0);

  const bool inaAck = i2cAck(Wire1, inaAddress);
  uint16_t manufacturerId = 0;
  uint16_t dieId = 0;
  const bool manufacturerRead = readRegister16(Wire1, inaAddress, 0xFE, manufacturerId);
  const bool dieRead = readRegister16(Wire1, inaAddress, 0xFF, dieId);
  
  // Initialize the INA3221
  if (!ina3221.begin(inaAddress, &Wire1)) {
    const int foundAddress = findFirstI2CAddress(Wire1);
    canvas.setCursor(x, y);
    canvas.printf("INA3221 not found");
    y += lineHeight;
    canvas.setCursor(x, y);
    canvas.printf("SDA=%d SCL=%d ACK=%d", i2cSda, i2cScl, inaAck ? 1 : 0);
    y += lineHeight;
    canvas.setCursor(x, y);
    canvas.printf("Addr 0x%02X M=%04X D=%04X", inaAddress,
                  manufacturerRead ? manufacturerId : 0,
                  dieRead ? dieId : 0);
    y += lineHeight;
    canvas.setCursor(x, y);
    if (foundAddress >= 0) {
      canvas.printf("First I2C dev: 0x%02X", foundAddress);
    } else {
      canvas.printf("No I2C devices seen");
    }
    canvas.pushSprite(0, 0);
    while (1) {
      delay(10);
    }
  }
  canvas.setCursor(x, y + lineHeight);
  canvas.printf("INA3221 found @0x%02X", inaAddress);

  // Setup sensor
  ina3221.setAveragingMode(INA3221_AVG_16_SAMPLES);
  for (uint8_t i = 0; i < 3; i++) {
    ina3221.setShuntResistance(i, 0.05); // Set shunt resistances for all channels to 0.05 ohms
  }
  ina3221.setPowerValidLimits(3.0 /* lower limit */, 15.0 /* upper limit */); // Set a power valid alert to tell us if ALL channels are between the two limits
 
  // Make canvas visible on LCD
  canvas.pushSprite(0, 0);
  delay(1000);

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
