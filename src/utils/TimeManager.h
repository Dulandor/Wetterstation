#pragma once

#include <NTPClient.h>
#include <WiFiUdp.h>

class TimeManager {
private:
  unsigned long lastNTPUpdate = 0;
  unsigned long lastDataSave = 0;
  unsigned long timeOffset = 0;
  bool initialized = false;
  NTPClient& timeClient;
  static const unsigned long NTP_UPDATE_INTERVAL = 3600000;  // 1 hour
  static const unsigned long DATA_SAVE_INTERVAL = 300000;    // 5 minutes

public:
  TimeManager(NTPClient& client) : timeClient(client) {}
  
  bool isInitialized() const { return initialized; }
  void setInitialized(bool value) { initialized = value; }
  unsigned long getTimeOffset() const { return timeOffset; }
  void setTimeOffset(unsigned long offset) { timeOffset = offset; }
  unsigned long getLastNTPUpdate() const { return lastNTPUpdate; }
  void setLastNTPUpdate(unsigned long time) { lastNTPUpdate = time; }
  unsigned long getLastDataSave() const { return lastDataSave; }
  void setLastDataSave(unsigned long time) { lastDataSave = time; }
  
  bool syncTime();
  void update();
  unsigned long getCurrentTimestamp() const;
  String getFormattedTimestamp() const;
  bool isSummerTime() const;
}; 