#pragma once

#include <WiFi101.h>
#include "SensorManager.h"
#include "TimeManager.h"

class WebServer {
private:
  WiFiServer server;
  SensorManager& sensorManager;
  TimeManager& timeManager;
  
  void sendDashboardResponse(WiFiClient& client);
  void sendLiveDataResponse(WiFiClient& client);
  String getCurrentData(const SensorData& data);
  String getDashboardData();
  String readHistoricalData();

public:
  WebServer(SensorManager& sensors, TimeManager& time) 
    : server(80), sensorManager(sensors), timeManager(time) {}
    
  void begin() { server.begin(); }
  void handleClient();
}; 