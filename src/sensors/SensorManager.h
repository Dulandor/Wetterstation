#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_HDC1000.h>
#include <Adafruit_DPS310.h>
#include <LTR329.h>
#include <VEML6070.h>

// Data structure for sensor readings
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
public:
  SensorManager(Adafruit_HDC1000& hdc, Adafruit_DPS310& dps, VEML6070& veml, LTR329& ltr);
  bool initialize();
  SensorData readData();
  int read_reg(byte address, uint8_t reg);
  void write_reg(byte address, uint8_t reg, uint8_t val);
  void Lightsensor_begin();
  uint32_t Lightsensor_getIlluminance();

private:
  Adafruit_HDC1000& hdc;
  Adafruit_DPS310& dps;
  VEML6070& veml;
  LTR329& ltr;
  bool lightsensortype; // 0 for TSL, 1 for LTR
  unsigned char gain;
  unsigned char integrationTime;
  unsigned char measurementRate;
}; 