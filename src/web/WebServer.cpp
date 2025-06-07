#include "WebServer.h"
#include <SD.h>

void WebServer::handleClient() {
  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";
    String request = "";
    unsigned long timeout = millis() + 5000;
    
    while (client.connected() && millis() < timeout) {
      if (client.available()) {
        char c = client.read();
        request += c;
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (request.indexOf("GET /dashboard") >= 0) {
              sendDashboardResponse(client);
            } else {
              sendLiveDataResponse(client);
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
  }
}

void WebServer::sendDashboardResponse(WiFiClient& client) {
  String dashboardData = getDashboardData();
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  const int chunkSize = 512;
  for (int i = 0; i < dashboardData.length(); i += chunkSize) {
    String chunk = dashboardData.substring(i, min(i + chunkSize, dashboardData.length()));
    client.print(chunk);
    delay(10);
  }
}

void WebServer::sendLiveDataResponse(WiFiClient& client) {
  SensorData data = sensorManager.readData();
  String liveData = getCurrentData(data);
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.print(liveData);
}

String WebServer::getCurrentData(const SensorData& data) {
  String html = "<!DOCTYPE html>";
  html += "<html lang='de'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='30'>";
  html += "<title>Weather Station Live Data</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#f0f2f5}";
  html += ".container{max-width:800px;margin:0 auto}";
  html += ".card{background:white;border-radius:10px;padding:20px;margin-bottom:20px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}";
  html += "h1{color:#1a73e8;margin-bottom:20px}";
  html += "table{width:100%;border-collapse:collapse}";
  html += "td,th{padding:12px;text-align:left;border-bottom:1px solid #ddd}";
  html += "th{background:#f8f9fa}";
  html += ".nav{margin-bottom:20px}";
  html += ".nav a{color:#1a73e8;text-decoration:none;margin-right:20px}";
  html += "</style></head><body>";
  html += "<div class='container'><div class='nav'>";
  html += "<a href='/'>Live Data</a><a href='/dashboard'>Dashboard</a></div>";
  html += "<div class='card'><h1>Weather Station Live Data</h1>";
  html += "<table><tr><th>Sensor</th><th>Value</th><th>Unit</th></tr>";
  html += "<tr><td>Temperature</td><td>" + String(data.temperature, 2) + "</td><td>°C</td></tr>";
  html += "<tr><td>Humidity</td><td>" + String(data.humidity, 2) + "</td><td>%</td></tr>";
  html += "<tr><td>Pressure</td><td>" + String(data.pressure, 2) + "</td><td>hPa</td></tr>";
  html += "<tr><td>Altitude</td><td>" + String(data.altitude, 2) + "</td><td>m</td></tr>";
  html += "<tr><td>Light</td><td>" + String(data.light) + "</td><td>lux</td></tr>";
  html += "<tr><td>UV Intensity</td><td>" + String(data.uv) + "</td><td>µW/cm²</td></tr>";
  html += "</table></div></div></body></html>";
  return html;
}

String WebServer::getDashboardData() {
  String historicalData = readHistoricalData();
  
  if (historicalData.length() == 0) {
    return "<!DOCTYPE html><html><body><h1>No data available</h1></body></html>";
  }

  String html = "<!DOCTYPE html><html lang='de'><head>";
  html += "<meta charset='UTF-8'><meta http-equiv='refresh' content='30'>";
  html += "<title>Weather Station Dashboard</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#f0f2f5}";
  html += ".container{max-width:800px;margin:0 auto}";
  html += ".card{background:white;border-radius:10px;padding:20px;margin-bottom:20px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}";
  html += "h1{color:#1a73e8;margin-bottom:20px}";
  html += ".nav{margin-bottom:20px}";
  html += ".nav a{color:#1a73e8;text-decoration:none;margin-right:20px}";
  html += "table{width:100%;border-collapse:collapse;margin-top:20px}";
  html += "td,th{padding:12px;text-align:left;border-bottom:1px solid #ddd}";
  html += "th{background:#f8f9fa}";
  html += "</style></head><body>";
  html += "<div class='container'><div class='nav'>";
  html += "<a href='/'>Live Data</a><a href='/dashboard'>Dashboard</a></div>";
  html += "<div class='card'><h1>Weather Station Dashboard</h1>";
  html += "<h2>Sampled Data Points</h2>";
  html += "<table><tr><th>Timestamp</th><th>Temp</th><th>Hum</th><th>Press</th><th>Alt</th><th>Light</th><th>UV</th></tr>";

  int startPos = 0;
  while (startPos < historicalData.length()) {
    int endPos = historicalData.indexOf('\n', startPos);
    if (endPos == -1) break;
    
    String line = historicalData.substring(startPos, endPos);
    String values[7];
    int valueIndex = 0;
    int lastComma = -1;
    
    for (int i = 0; i < line.length(); i++) {
      if (line[i] == ',' || i == line.length() - 1) {
        values[valueIndex++] = line.substring(lastComma + 1, i + (i == line.length() - 1 ? 1 : 0));
        lastComma = i;
      }
    }
    
    if (valueIndex == 7) {
      html += "<tr>";
      for (int i = 0; i < 7; i++) {
        html += "<td>" + values[i] + "</td>";
      }
      html += "</tr>";
    }
    
    startPos = endPos + 1;
  }
  
  html += "</table></div></div></body></html>";
  return html;
}

String WebServer::readHistoricalData() {
  if (!SD.exists("data.txt")) {
    return "";
  }

  File dataFile = SD.open("data.txt", FILE_READ);
  if (!dataFile) {
    return "";
  }

  // Skip header line
  dataFile.readStringUntil('\n');

  // Count total lines
  int totalLines = 0;
  while (dataFile.available()) {
    if (dataFile.readStringUntil('\n').length() > 0) {
      totalLines++;
    }
  }

  // Calculate sampling interval
  const int NUM_POINTS = 10;
  int interval = max(1, totalLines / NUM_POINTS);

  // Reset file position
  dataFile.seek(0);
  dataFile.readStringUntil('\n');

  // Read sampled lines
  String data = "";
  int currentLine = 0;
  int sampledLines = 0;
  
  while (dataFile.available() && sampledLines < NUM_POINTS) {
    String line = dataFile.readStringUntil('\n');
    if (line.length() > 0) {
      if (currentLine % interval == 0) {
        data += line + "\n";
        sampledLines++;
      }
      currentLine++;
    }
  }

  dataFile.close();
  return data;
} 