#include "test_wifi.h"

bool disconnectWiFi() {
  WiFi.disconnect();
  return true;
}

bool sendToServer(const char* test_id, bool productA_pressed, bool productB_pressed, unsigned long timestamp) {
  if (WiFi.status() != WL_CONNECTED) {
    LOG_INFO(">>> SERVER: No WiFi connection");
    return false;
  }
 
  WiFiClient client;
  String host = SECRET_IP;
  int port = 3000;
 
  if (!client.connect(host.c_str(), port)) {
    LOG_INFO(">>> SERVER: Connection failed");
    return false;
  }
 
  // escapeJsonString from csvToJson.h
  String jsonData = "{\"test_id\":\"" + escapeJsonString(test_id) + "\",";
  jsonData += "\"data\":{\"productA\":" + String(productA_pressed ? "true" : "false") + ",";
  jsonData += "\"productB\":" + String(productB_pressed ? "true" : "false") + "},";
  jsonData += "\"device_id\":\"" + escapeJsonString(DEVICE_ID) + "\",";
  jsonData += "\"timestamp\":" + String(timestamp) + "}";
 
  String request = "POST /api/tests HTTP/1.1\r\n";
  request += "Host: " + host + ":" + String(port) + "\r\n";
  request += "Content-Type: application/json\r\n";
  request += "Content-Length: " + String(jsonData.length()) + "\r\n";
  request += "Connection: close\r\n\r\n";
  request += jsonData;
 
  client.print(request);
 
  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 5000) {
    if (client.available()) {
      String response = client.readStringUntil('\n');
      if (response.indexOf("HTTP/1.1 200") != -1) {
        LOG_INFO(">>> SERVER: Data sent successfully");
        client.stop();
        return true;
      }
    }
  }
 
  LOG_ERROR(">>> SERVER: No valid response");
  client.stop();
  return false;
}

void logWiFiStatus() {
  RETURN_IF_NOT_DEBUG;
  uint8_t status = WiFi.status();
  Serial.print(">>> WiFi Status: ");
  switch(status) {
    case WL_NO_SHIELD: Serial.println("No shield"); break;
    case WL_IDLE_STATUS: Serial.println("Idle"); break;
    case WL_NO_SSID_AVAIL: Serial.println("SSID not available"); break;
    case WL_CONNECTED: Serial.println("Connected"); break;
    case WL_CONNECT_FAILED: Serial.println("Connection failed"); break;
    case WL_CONNECTION_LOST: Serial.println("Connection lost"); break;
    case WL_DISCONNECTED: Serial.println("Disconnected"); break;
    default: Serial.println("Unknown"); break;
  }
}

bool isWiFiConnecting() {
  return WiFi.status() == WL_IDLE_STATUS;
}
