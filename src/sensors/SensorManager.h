#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_HDC1000.h>
#include <Adafruit_DPS310.h>
#include <LTR329.h>
#include <VEML6070.h>

struct SensorData {
  float temperature;    // Temperature in °C
  float humidity;       // Humidity in %
  float pressure;       // Pressure in hPa
  float altitude;       // Altitude in m
  uint32_t light;       // Light intensity in lux
  float uv;            // UV radiation
  unsigned long timestamp; // Timestamp
};

class SensorManager {
private:
  Adafruit_HDC1000& hdc;
  Adafruit_DPS310& dps;
  VEML6070& veml;
  LTR329& ltr;
  bool lightsensortype; // 0 = TSL, 1 = LTR

  int read_reg(byte address, uint8_t reg);
  void write_reg(byte address, uint8_t reg, uint8_t val);
  void Lightsensor_begin();
  uint32_t Lightsensor_getIlluminance();

public:
  SensorManager(Adafruit_HDC1000& h, Adafruit_DPS310& d, VEML6070& v, LTR329& l) 
    : hdc(h), dps(d), veml(v), ltr(l), lightsensortype(0) {}
  
  bool initialize();
  SensorData readData();
}; 