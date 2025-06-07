#include "SensorManager.h"
#include <Wire.h>

SensorManager::SensorManager(Adafruit_HDC1000& h, Adafruit_DPS310& d, VEML6070& v, LTR329& l) 
  : hdc(h), dps(d), veml(v), ltr(l), lightsensortype(0) {}

bool SensorManager::initialize() {
  bool success = true;
  
  if (!hdc.begin()) {
    Serial.println("HDC1000 not detected");
    success = false;
  }
  
  if (!dps.begin_I2C(0x76)) {
    Serial.println("DPS310 not detected");
    success = false;
  } else {
    dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }
  
  Lightsensor_begin();
  veml.begin();
  
  return success;
}

SensorData SensorManager::readData() {
  sensors_event_t temp_event, pressure_event;
  dps.getEvents(&temp_event, &pressure_event);
  
  return {
    hdc.readTemperature(),
    hdc.readHumidity(),
    pressure_event.pressure,
    dps.readAltitude(1013),
    Lightsensor_getIlluminance(),
    veml.getUV(),
    millis() / 1000  // Basic timestamp, will be updated by TimeManager
  };
}

int SensorManager::read_reg(byte address, uint8_t reg) {
  int i = 0;
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)1);
  delay(1);
  if(Wire.available()) i = Wire.read();
  return i;
}

void SensorManager::write_reg(byte address, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void SensorManager::Lightsensor_begin() {
  unsigned int u = read_reg(0x29, 0x80 | 0x0A);
  if ((u & 0xF0) == 0xA0) {  // TSL45315
    write_reg(0x29, 0x80 | 0x00, 0x03);
    write_reg(0x29, 0x80 | 0x01, 0x02);
    delay(120);
    lightsensortype = 0;
  } else {  // LTR329
    LTR.begin();
    LTR.setControl(1, false, false);
    LTR.setMeasurementRate(0, 3);
    LTR.setPowerUp();
    delay(10);
    lightsensortype = 1;
  }
}

uint32_t SensorManager::Lightsensor_getIlluminance() {
  if (lightsensortype == 0) {  // TSL45315
    unsigned int u = (read_reg(0x29, 0x80 | 0x04) << 0);
    u |= (read_reg(0x29, 0x80 | 0x05) << 8);
    return u * 4;
  } else {  // LTR329
    delay(100);
    unsigned int data0, data1;
    unsigned int lux = 0;
    for (int i = 0; i < 5; i++) {
      if (LTR.getData(data0, data1)) {
        if(LTR.getLux(1, 0, data0, data1, lux) && lux > 0) break;
        delay(10);
      }
    }
    return lux;
  }
} 