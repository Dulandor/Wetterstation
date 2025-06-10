#include "SensorManager.h"
#include <Wire.h>

SensorManager::SensorManager(Adafruit_HDC1000& h, Adafruit_DPS310& d, VEML6070& v, LTR329& l) 
  : hdc(h), dps(d), veml(v), ltr(l), lightsensortype(0) {}

bool SensorManager::initialize() {
  bool success = true;
  
  // Initialize temperature and humidity sensor
  if (!hdc.begin()) {
    Serial.println("HDC1000 not detected");
    success = false;
  }
  
  // Initialize pressure sensor with I2C address 0x76
  if (!dps.begin_I2C(0x76)) {
    Serial.println("DPS310 not detected");
    success = false;
  } else {
    // Configure for high precision measurements
    dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }
  
  // Initialize light sensor (auto-detects LTR329 or TSL45315)
  Lightsensor_begin();
  
  // Initialize UV sensor
  veml.begin();
  
  return success;
}

SensorData SensorManager::readData() {
  // Get pressure and temperature events from DPS310
  sensors_event_t temp_event, pressure_event;
  dps.getEvents(&temp_event, &pressure_event);
  
  // Collect all sensor readings into a structured data object
  return {
    hdc.readTemperature(),           // Temperature in °C
    hdc.readHumidity(),              // Humidity in %
    pressure_event.pressure,         // Pressure in hPa
    dps.readAltitude(1013),          // Altitude in meters (sea level pressure: 1013 hPa)
    Lightsensor_getIlluminance(),    // Light intensity in lux
    veml.getUV(),                    // UV radiation in µW/cm²
    0                                // Timestamp will be set by the caller
  };
}

// Low-level I2C register read function for light sensor communication
int SensorManager::read_reg(byte address, uint8_t reg) {
  int i = 0;
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)1);
  delay(1);
  if(Wire.available())
    i = Wire.read();
  return i;
}

// Low-level I2C register write function for light sensor communication
void SensorManager::write_reg(byte address, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

// Auto-detect and initialize light sensor (TSL45315 or LTR329)
void SensorManager::Lightsensor_begin() {
  unsigned int u = 0;
  u = read_reg(0x29, 0x80 | 0x0A); // Read ID register to detect sensor type
  
  if ((u & 0xF0) == 0xA0) {         // TSL45315 detected
    write_reg(0x29, 0x80 | 0x00, 0x03); // Control: power on
    write_reg(0x29, 0x80 | 0x01, 0x02); // Config: M=4, T=100ms
    delay(120);                          // Wait for sensor to stabilize
    lightsensortype = 0;                 // Set flag for TSL45315
  } else {                               // LTR-329ALS-01 assumed
    ltr.begin();
    ltr.setControl(1, false, false);     // Set gain = 1
    ltr.setMeasurementRate(0, 3);        // Integration time = 0, measurement rate = 3
    ltr.setPowerUp();                    // Power on with default settings
    delay(10);                           // Wait for wakeup from standby
    lightsensortype = 1;                 // Set flag for LTR-329ALS-01
  }
}

// Read illuminance value from the detected light sensor
uint32_t SensorManager::Lightsensor_getIlluminance() {
  unsigned int lux = 0;
  
  if (lightsensortype == 0) {  // TSL45315 sensor
    // Read 16-bit data from sensor registers
    unsigned int u = (read_reg(0x29, 0x80 | 0x04) << 0);  // Data low byte
    u |= (read_reg(0x29, 0x80 | 0x05) << 8);             // Data high byte
    lux = u * 4;  // Calculate lux with multiplier M=4 and integration time T=100ms
    
  } else if (lightsensortype == 1) {  // LTR-329ALS-01 sensor
    delay(100);  // Allow time for measurement
    unsigned int data0, data1;
    
    // Retry up to 5 times to get valid reading
    for (int i = 0; i < 5; i++) {
      if (ltr.getData(data0, data1)) {
        if(ltr.getLux(1, 0, data0, data1, lux)) {
          if(lux > 0) break;  // Valid reading obtained
        }
        delay(10);
      }
    }
  }
  return lux;
}