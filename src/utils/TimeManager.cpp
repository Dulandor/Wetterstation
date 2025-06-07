#include "TimeManager.h"
#include <WiFi101.h>

bool TimeManager::syncTime() {
  Serial.println("🕒 Attempting NTP time sync...");
  
  if (timeClient.update()) {
    timeOffset = timeClient.getEpochTime() * 1000 - millis();
    initialized = true;
    Serial.println("✅ Time sync successful");
    return true;
  }
  
  Serial.println("❌ Time sync failed");
  return false;
}

void TimeManager::update() {
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - lastNTPUpdate > NTP_UPDATE_INTERVAL) {
      if (syncTime()) {
        lastNTPUpdate = millis();
      }
    }
  }
}

unsigned long TimeManager::getCurrentTimestamp() const {
  if (initialized) {
    return (millis() + timeOffset) / 1000;
  }
  return millis() / 1000;
}

String TimeManager::getFormattedTimestamp() const {
  if (!initialized) {
    unsigned long currentMillis = millis() / 1000;
    int hour = (currentMillis / 3600) % 24;
    int minute = (currentMillis / 60) % 60;
    int second = currentMillis % 60;
    
    char buffer[30];
    sprintf(buffer, "00.00.0000;%02d:%02d:%02d", hour, minute, second);
    return String(buffer);
  }

  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  
  if (isSummerTime()) {
    epochTime += 3600;
  }
  
  int year = 1970;
  int month = 1;
  int day = 1;
  
  while (epochTime >= 31536000) {
    epochTime -= 31536000;
    year++;
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
      epochTime -= 86400;
    }
  }
  
  int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
    daysInMonth[1] = 29;
  }
  
  while (epochTime >= 86400) {
    if (epochTime >= daysInMonth[month - 1] * 86400) {
      epochTime -= daysInMonth[month - 1] * 86400;
      month++;
    } else {
      break;
    }
  }
  
  day = 1 + (epochTime / 86400);
  epochTime %= 86400;
  
  int hour = epochTime / 3600;
  epochTime %= 3600;
  int minute = epochTime / 60;
  int second = epochTime % 60;
  
  char buffer[30];
  sprintf(buffer, "%02d.%02d.%04d;%02d:%02d:%02d", 
    day, month, year, hour, minute, second);
  return String(buffer);
}

bool TimeManager::isSummerTime() const {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  
  int year = 1970 + (epochTime / 31536000);
  int month = 1 + ((epochTime % 31536000) / 2592000);
  int day = 1 + ((epochTime % 2592000) / 86400);
  
  if (month > 3 && month < 10) return true;
  if (month == 3) {
    int lastSunday = 31 - ((5 + year + year/4) % 7);
    if (day >= lastSunday) return true;
  }
  if (month == 10) {
    int lastSunday = 31 - ((5 + year + year/4) % 7);
    if (day < lastSunday) return true;
  }
  return false;
} 