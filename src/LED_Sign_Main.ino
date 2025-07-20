/*
 * LED Sign - WiFi Configuration Portal for ESP32-C3
 * Fixed version with proper initialization
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>  // Add JSON library

// ESP32 specific includes
extern "C" {
  #include "esp_wifi.h"
  #include "esp_system.h"
  #include <esp_wifi_types.h>
  #include <esp_netif.h>
}

// Configuration
#define AP_SSID          "LED-Sign-Setup"    // AP name
#define AP_PASSWORD      "configure123"       // At least 8 characters
#define DNS_PORT         53
#define WEB_PORT         80
#define STATUS_LED_PIN   8
#define WIFI_CHANNEL     6                   // Use channel 6 for better stability
#define MAX_CLIENTS      4                   // Maximum number of clients

// Objects
WebServer server(WEB_PORT);
DNSServer dnsServer;
Preferences preferences;

// State variables
bool wifiConfigured = false;
String savedSSID = "";
String savedPassword = "";

void setup() {
  // Critical: Initialize hardware first
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);
  
  // Initialize Serial with proper timing for ESP32-C3
  Serial.begin(115200);
  while (!Serial && millis() < 5000); // Wait up to 5 seconds
  delay(1000); // Extra delay for USB CDC to stabilize
  
  Serial.println("\n\n=====================================");
  Serial.println("LED Sign WiFi Portal - ESP32-C3");
  Serial.println("=====================================");

  // Initialize SPIFFS with formatting if needed
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed - Critical Error");
    blinkLED(10, 100); // Error indicator
    delay(1000);
    ESP.restart();
    return;
  }
  
  // List files in SPIFFS
  Serial.println("\nSPIFFS Files:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file) {
    Serial.print("  ");
    Serial.print(file.name());
    Serial.print("  -  ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
  Serial.println("SPIFFS Mounted Successfully");
  
  // Critical: Properly initialize WiFi subsystem for ESP32-C3
  WiFi.mode(WIFI_MODE_NULL);
  delay(100);
  
  // Load saved credentials
  preferences.begin("wifi-config", false);
  savedSSID = preferences.getString("ssid", "");
  savedPassword = preferences.getString("password", "");
  
  if (savedSSID.length() > 0) {
    Serial.println("Found saved WiFi credentials");
    connectToWiFi();
  } else {
    Serial.println("No saved credentials, starting setup portal");
    startConfigPortal();
  }
}

void startConfigPortal() {
  Serial.println("\nStarting Configuration Portal...");
  
  // Complete WiFi shutdown and cleanup
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  
  // Initialize WiFi with full power
  esp_wifi_set_max_tx_power(78);  // Set to maximum power (20dBm)
  WiFi.mode(WIFI_AP);
  delay(100);
  
  // Configure AP with static IP - Using 10.10.10.x range to avoid conflicts
  IPAddress local_IP(10, 10, 10, 1);
  IPAddress gateway(10, 10, 10, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  Serial.println("Configuring AP...");
  Serial.print("Setting Soft-AP configuration ... ");
  
  // Set WiFi country for proper channel operation
  esp_wifi_set_country_code("US", true);
  delay(100);
  
  // Configure the AP with static IP
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("AP Config Failed!");
    blinkLED(5, 100); // Error indicator
    delay(1000);
    ESP.restart(); // Restart on failure
    return;
  }
  
  Serial.println("AP Config Success!");
  
  // Start Access Point with specific channel and max clients
  Serial.print("Starting AP with SSID: ");
  Serial.println(AP_SSID);
  
  WiFi.softAPdisconnect(true);
  delay(500);
  
  // Start AP with hidden SSID disabled and max connections
  if (WiFi.softAP(AP_SSID, AP_PASSWORD, WIFI_CHANNEL, 0, MAX_CLIENTS)) {
    delay(500); // Let it stabilize
    
    // Verify AP started correctly
    IPAddress myIP = WiFi.softAPIP();
    if (myIP == local_IP) {
      Serial.println("Access Point Started Successfully!");
      Serial.print("SSID: ");
      Serial.println(AP_SSID);
      Serial.print("Password: ");
      Serial.println(AP_PASSWORD);
      Serial.print("Channel: ");
      Serial.println(WIFI_CHANNEL);
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      Serial.print("AP MAC address: ");
      Serial.println(WiFi.softAPmacAddress());
      
      // Print DHCP range
      Serial.println("DHCP Server Range:");
      Serial.println("Start IP: 10.10.10.2");
      Serial.println("End IP: 10.10.10.254");
      
      // Get and print current power settings
      int8_t power;
      esp_wifi_get_max_tx_power(&power);
      Serial.print("TX Power: ");
      Serial.println(power);
      
      // Start DNS server for captive portal
      dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
      dnsServer.start(DNS_PORT, "*", local_IP);
      
      // Setup web server
      setupWebServer();
      server.begin();
      
      Serial.println("\nPortal ready at http://10.10.10.1");
      blinkLED(3, 200); // 3 blinks = portal ready
    } else {
      Serial.println("AP IP configuration failed!");
      Serial.print("Expected IP: ");
      Serial.println(local_IP);
      Serial.print("Actual IP: ");
      Serial.println(myIP);
      blinkLED(5, 100);
      delay(1000);
      ESP.restart();
    }
  } else {
    Serial.println("Failed to start AP!");
    Serial.println("WiFi Status: " + String(WiFi.status()));
    Serial.println("Retrying in 5 seconds...");
    blinkLED(5, 100); // 5 fast blinks = error
    delay(5000);
    ESP.restart(); // Restart on failure
  }
}

void connectToWiFi() {
  Serial.print("\nConnecting to WiFi: ");
  Serial.println(savedSSID);
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  delay(100);
  
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 60) {  // Increased timeout to 30 seconds
    delay(500);
    Serial.print(".");
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    
    wifiConfigured = true;
    digitalWrite(STATUS_LED_PIN, LOW); // LED off = connected
    
    // Setup web server with SPIFFS support
    setupWebServer();
    
    // Add reset endpoint
    server.on("/reset", []() {
      preferences.clear();
      server.send(200, "text/html", "<h1>Settings Reset</h1><p>Restarting...</p>");
      delay(1000);
      ESP.restart();
    });
    
    server.begin();
    Serial.println("Web server started at " + WiFi.localIP().toString());
  } else {
    Serial.println("\nConnection failed!");
    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());
    startConfigPortal();
  }
}

// Helper function to escape JSON strings
String escapeJSON(const String& str) {
  String escaped = "";
  for (char c : str) {
    switch (c) {
      case '\"': escaped += "\\\""; break;
      case '\\': escaped += "\\\\"; break;
      case '\b': escaped += "\\b"; break;
      case '\f': escaped += "\\f"; break;
      case '\n': escaped += "\\n"; break;
      case '\r': escaped += "\\r"; break;
      case '\t': escaped += "\\t"; break;
      default:
        if (c < 0x20) {
          char buf[7];
          snprintf(buf, sizeof(buf), "\\u%04x", c);
          escaped += buf;
        } else {
          escaped += c;
        }
    }
  }
  return escaped;
}

void setupWebServer() {
  // API endpoints
  server.on("/api/status", HTTP_GET, []() {
    DynamicJsonDocument doc(512);
    doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
    doc["wifi_ssid"] = WiFi.SSID();
    doc["ip_address"] = WiFi.localIP().toString();
    doc["mac_address"] = WiFi.macAddress();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    
    String response;
    serializeJson(doc, response);
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, F("application/json"), response);
  });

  server.on("/api/wifi/scan", HTTP_GET, []() {
    WiFi.scanNetworks(true);
    DynamicJsonDocument doc(128);
    doc["status"] = "scanning";
    String response;
    serializeJson(doc, response);
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, F("application/json"), response);
  });

  server.on("/api/scanresults", HTTP_GET, []() {
    DynamicJsonDocument doc(4096);
    JsonArray networks = doc.createNestedArray("networks");
    
    int n = WiFi.scanComplete();
    if (n > 0) {
      for (int i = 0; i < n; i++) {
        JsonObject network = networks.createNestedObject();
        network["ssid"] = WiFi.SSID(i);
        network["rssi"] = WiFi.RSSI(i);
        network["encryption"] = WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "WPA2";
      }
      WiFi.scanDelete();
      WiFi.scanNetworks(true);
    }
    
    String response;
    serializeJson(doc, response);
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, F("application/json"), response);
  });

  server.on("/api/wifi/connect", HTTP_POST, []() {
    if (!server.hasArg("ssid") || !server.hasArg("password")) {
      server.send(400, F("application/json"), F("{\"status\":\"error\",\"message\":\"Missing credentials\"}"));
      return;
    }
    
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    if (ssid.length() == 0 || password.length() < 8) {
      server.send(400, F("application/json"), F("{\"status\":\"error\",\"message\":\"Invalid credentials\"}"));
      return;
    }
    
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    
    server.send(200, F("application/json"), F("{\"status\":\"connected\"}"));
    delay(500);
    ESP.restart();
  });

  // Handle static files from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "File Not Found");
    }
  });
  
  // Start server
  server.begin();
  Serial.println("\nWeb server started");
}

bool handleFileRead(String path) {
  Serial.print("handleFileRead: ");
  Serial.println(path);

  if (path.endsWith("/")) {
    path += "index.html";
  }
  
  String contentType;
  if (path.endsWith(".html")) {
    contentType = F("text/html");
  } else if (path.endsWith(".css")) {
    contentType = F("text/css");
  } else if (path.endsWith(".js")) {
    contentType = F("application/javascript");
  } else if (path.endsWith(".json")) {
    contentType = F("application/json");
  } else {
    contentType = F("text/plain");
  }

  // Keep the leading slash for SPIFFS
  String spiffsPath = path;
  if (!spiffsPath.startsWith("/")) {
    spiffsPath = "/" + path;
  }
  
  Serial.print("SPIFFS path: ");
  Serial.println(spiffsPath);
  
  if (SPIFFS.exists(spiffsPath)) {
    Serial.println("File exists");
    File file = SPIFFS.open(spiffsPath, "r");
    if (!file) {
      Serial.println("Failed to open file");
      return false;
    }
    
    size_t fileSize = file.size();
    Serial.print("File size: ");
    Serial.println(fileSize);
    
    server.sendHeader("Content-Length", String(fileSize));
    server.sendHeader("Cache-Control", "no-cache");
    
    if (server.streamFile(file, contentType) != fileSize) {
      Serial.println("Sent less data than expected!");
    } else {
      Serial.println("File sent successfully");
    }
    
    file.close();
    return true;
  }
  
  Serial.println("File not found");
  return false;
}

// Helper function to get content type based on file extension
String getContentType(String filename) {
  if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(delayMs);
  }
}

void loop() {
  if (!wifiConfigured) {
    // Process DNS requests more frequently
    for(int i = 0; i < 10; i++) {
      dnsServer.processNextRequest();
      server.handleClient();  // Handle web requests more frequently in AP mode
      delay(1);
    }
    
    // Monitor AP status
    static unsigned long lastAPCheck = 0;
    if (millis() - lastAPCheck > 5000) { // Check every 5 seconds
      lastAPCheck = millis();
      
      // Verify AP is still running
      if (WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
        Serial.println("AP mode lost! Restarting...");
        delay(1000);
        ESP.restart();
      }
      
      // Print AP status
      Serial.println("\nAP Status:");
      Serial.print("Channel: ");
      Serial.println(WiFi.channel());
      Serial.print("Connected clients: ");
      Serial.println(WiFi.softAPgetStationNum());
      
      // Print client details if any are connected
      wifi_sta_list_t stationList;
      esp_netif_sta_list_t adapterList;
      esp_wifi_ap_get_sta_list(&stationList);
      esp_netif_get_sta_list(&stationList, &adapterList);
      
      for(int i = 0; i < adapterList.num; i++) {
        esp_netif_sta_info_t station = adapterList.sta[i];
        Serial.print("Client ");
        Serial.print(i + 1);
        Serial.print(" MAC: ");
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                station.mac[0], station.mac[1], station.mac[2],
                station.mac[3], station.mac[4], station.mac[5]);
        Serial.print(macStr);
        Serial.print(" IP: ");
        Serial.println(IPAddress(station.ip.addr));
      }
      
      // Get and print current power settings
      int8_t power;
      esp_wifi_get_max_tx_power(&power);
      Serial.print("Current TX Power: ");
      Serial.println(power);
    }
  } else {
    server.handleClient();
  }
  
  // Status LED
  static unsigned long lastBlink = 0;
  if (!wifiConfigured && millis() - lastBlink > 2000) {
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    lastBlink = millis();
  }
  
  delay(1); // Small delay to prevent watchdog issues
}

 