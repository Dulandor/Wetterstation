/*
 * Weather Station Project
 * 
 * This program controls a weather station with the following sensors:
 * - Temperature and Humidity (HDC1000)
 * - Pressure (DPS310)
 * - Light Intensity (LTR329 or TSL45315)
 * - UV Radiation (VEML6070)
 * 
 * Features:
 * - Data logging to SD card
 * - Web server for real-time data access
 * - NTP time synchronization
 * - WiFi connectivity
 * 
 * Author: Leander
 * Version: 1.0
 */

// ===== INCLUDES =====
#include <Arduino.h>
#include <senseBoxIO.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Forward declarations
class Adafruit_HDC1000;
class Adafruit_DPS310;
class LTR329;
class VEML6070;

#include "src/sensors/SensorManager.h"
#include "src/utils/TimeManager.h"
#include "src/web/WebServer.h"

// ===== CONSTANTS =====
const int SD_CS_PIN = 28;
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const unsigned long DATA_SAVE_INTERVAL = 300000;  // 5 minutes
const int MAX_CONNECTION_ATTEMPTS = 3;

// ===== GLOBAL OBJECTS =====
static Adafruit_HDC1000 hdc;
static Adafruit_DPS310 dps;
static LTR329 ltr;
static VEML6070 veml;
static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);

static SensorManager sensorManager(hdc, dps, veml, ltr);
static TimeManager timeManager(timeClient);
static WebServer webServer(sensorManager, timeManager);

// ===== SETUP =====
void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial && millis() < 3000) {
    delay(10);
  }
  Serial.println("\nWeather Station Initialization");
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("❌ SD card initialization failed!");
  } else {
    Serial.println("✅ SD card initialized");
    if (!SD.exists("data.txt")) {
      File dataFile = SD.open("data.txt", FILE_WRITE);
      if (dataFile) {
        dataFile.println("timestamp,temperature,humidity,pressure,altitude,light,uv");
        dataFile.close();
        Serial.println("✅ Created data.txt with headers");
      }
    }
  }

  // Check WiFi shield
  Serial.println("🔍 Checking WiFi shield...");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("❌ No WiFi shield found!");
  } else {
    Serial.println("✅ WiFi shield detected");
    // Try to connect to WiFi with multiple attempts
    for (int i = 0; i < MAX_CONNECTION_ATTEMPTS; i++) {
      Serial.print("🔌 WiFi connection attempt ");
      Serial.print(i + 1);
      Serial.print(" of ");
      Serial.println(MAX_CONNECTION_ATTEMPTS);
      
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      unsigned long startAttemptTime = millis();
      
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
        delay(500);
        Serial.print(".");
      }
      Serial.println();
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi connected");
        IPAddress ip = WiFi.localIP();
        String ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
        Serial.print("📡 IP-Adresse: ");
        Serial.println(ipStr);
        webServer.begin();
        timeManager.syncTime();
        break;
      }
      
      // Wait longer between attempts
      delay(5000);
    }
  }

  // Initialize sensors
  if (sensorManager.initialize()) {
    Serial.println("✅ All sensors initialized successfully");
  } else {
    Serial.println("⚠️ Some sensors failed to initialize");
  }
  
  Serial.println("Setup complete");
}

// ===== LOOP =====
void loop() {
  // Update time and save data
  timeManager.update();
  if (millis() % DATA_SAVE_INTERVAL < 1000) {
    SensorData data = sensorManager.readData();
    data.timestamp = timeManager.getCurrentTimestamp();
    
    File dataFile = SD.open("data.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.print(timeManager.getFormattedTimestamp());
      dataFile.print(",");
      dataFile.print(data.temperature);
      dataFile.print(",");
      dataFile.print(data.humidity);
      dataFile.print(",");
      dataFile.print(data.pressure);
      dataFile.print(",");
      dataFile.print(data.altitude);
      dataFile.print(",");
      dataFile.print(data.light);
      dataFile.print(",");
      dataFile.println(data.uv);
      dataFile.close();
      Serial.println("✅ Data saved to SD card");
    }
  }
  
  // Handle web clients
  webServer.handleClient();
  
  delay(100);
}