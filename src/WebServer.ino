/*
 * Web Server and API Handlers for LED Sign
 * This file contains all web server routes and WebSocket handlers
 */

void initializeWebServer() {
  // Serve static files from SPIFFS
  webServer.onNotFound(handleFileRequest);
  
  // API Routes
  webServer.on("/api/status", HTTP_GET, handleGetStatus);
  webServer.on("/api/power", HTTP_POST, handleSetPower);
  webServer.on("/api/modes", HTTP_GET, handleGetModes);
  webServer.on("/api/mode", HTTP_POST, handleSetMode);
  webServer.on("/api/config", HTTP_GET, handleGetConfig);
  webServer.on("/api/config", HTTP_POST, handleSetConfig);
  webServer.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  webServer.on("/api/wifi/connect", HTTP_POST, handleWiFiConnect);
  webServer.on("/api/audio/level", HTTP_GET, handleGetAudioLevel);
  webServer.on("/api/reset", HTTP_POST, handleFactoryReset);
  
  // Root redirect for captive portal
  webServer.on("/", handleRoot);
  webServer.on("/generate_204", handleRoot); // Android captive portal
  webServer.on("/fwlink", handleRoot); // Microsoft captive portal
  
  webServer.begin();
  Serial.println("Web server started");
}

void initializeWebSocket() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started");
}

void handleRoot() {
  if (configMode) {
    // In config mode, serve setup page
    handleFileRequest();
  } else {
    // Normal mode, serve main interface
    handleFileRequest();
  }
}

void handleFileRequest() {
  String path = webServer.uri();
  
  // Default to index.html
  if (path.endsWith("/")) {
    path += "index.html";
  }
  
  // Determine content type
  String contentType = getContentType(path);
  
  // Try to serve from SPIFFS
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    webServer.streamFile(file, contentType);
    file.close();
  } else {
    // File not found, serve basic HTML if in config mode
    if (configMode) {
      webServer.send(200, "text/html", getSetupHTML());
    } else {
      webServer.send(200, "text/html", getMainHTML());
    }
  }
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

// API Handlers
void handleGetStatus() {
  StaticJsonDocument<512> doc;
  doc["power"] = systemOn;
  doc["mode"] = currentMode;
  doc["mode_name"] = modeNames[currentMode];
  doc["brightness"] = config.brightness;
  doc["speed"] = config.speed;
  doc["audio_level"] = audioLevel;
  doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
  doc["wifi_ssid"] = WiFi.SSID();
  doc["ip_address"] = WiFi.localIP().toString();
  doc["uptime"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  
  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleSetPower() {
  if (webServer.hasArg("power")) {
    systemOn = webServer.arg("power") == "true";
    
    if (!systemOn) {
      FastLED.clear();
      FastLED.show();
    }
    
    webServer.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    webServer.send(400, "application/json", "{\"error\":\"Missing power parameter\"}");
  }
}

void handleGetModes() {
  StaticJsonDocument<1024> doc;
  JsonArray modes = doc.createNestedArray("modes");
  
  for (int i = 0; i < MODE_COUNT; i++) {
    JsonObject mode = modes.createNestedObject();
    mode["id"] = i;
    mode["name"] = modeNames[i];
    mode["current"] = (i == currentMode);
  }
  
  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleSetMode() {
  if (webServer.hasArg("mode")) {
    int newMode = webServer.arg("mode").toInt();
    if (newMode >= 0 && newMode < MODE_COUNT) {
      currentMode = newMode;
      config.mode = currentMode;
      saveConfiguration();
      webServer.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      webServer.send(400, "application/json", "{\"error\":\"Invalid mode\"}");
    }
  } else {
    webServer.send(400, "application/json", "{\"error\":\"Missing mode parameter\"}");
  }
}

void handleGetConfig() {
  StaticJsonDocument<1024> doc;
  
  // LED Settings
  JsonObject led = doc.createNestedObject("led");
  led["brightness"] = config.brightness;
  led["speed"] = config.speed;
  led["mode"] = config.mode;
  led["color"] = {config.currentColor.r, config.currentColor.g, config.currentColor.b};
  
  // Audio Settings
  JsonObject audio = doc.createNestedObject("audio");
  audio["sensitivity"] = config.sensitivity;
  audio["noise_gate"] = config.noiseGate;
  audio["auto_gain"] = config.autoGain;
  audio["smoothing"] = config.smoothing;
  
  // System Settings
  JsonObject system = doc.createNestedObject("system");
  system["device_name"] = config.deviceName;
  system["timezone"] = config.timezone;
  system["sleep_timer"] = config.sleepTimer;
  system["wifi_timeout"] = config.wifiTimeout;
  
  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleSetConfig() {
  if (!webServer.hasArg("plain")) {
    webServer.send(400, "application/json", "{\"error\":\"No JSON payload\"}");
    return;
  }
  
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, webServer.arg("plain"));
  
  if (error) {
    webServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  // Update LED settings
  if (doc.containsKey("led")) {
    JsonObject led = doc["led"];
    if (led.containsKey("brightness")) config.brightness = led["brightness"];
    if (led.containsKey("speed")) config.speed = led["speed"];
    if (led.containsKey("mode")) {
      currentMode = led["mode"];
      config.mode = currentMode;
    }
    if (led.containsKey("color")) {
      JsonArray color = led["color"];
      config.currentColor = CRGB(color[0], color[1], color[2]);
    }
  }
  
  // Update audio settings
  if (doc.containsKey("audio")) {
    JsonObject audio = doc["audio"];
    if (audio.containsKey("sensitivity")) config.sensitivity = audio["sensitivity"];
    if (audio.containsKey("noise_gate")) config.noiseGate = audio["noise_gate"];
    if (audio.containsKey("auto_gain")) config.autoGain = audio["auto_gain"];
    if (audio.containsKey("smoothing")) config.smoothing = audio["smoothing"];
  }
  
  // Update system settings
  if (doc.containsKey("system")) {
    JsonObject system = doc["system"];
    if (system.containsKey("device_name")) {
      strlcpy(config.deviceName, system["device_name"], sizeof(config.deviceName));
    }
    if (system.containsKey("sleep_timer")) config.sleepTimer = system["sleep_timer"];
    if (system.containsKey("wifi_timeout")) config.wifiTimeout = system["wifi_timeout"];
  }
  
  saveConfiguration();
  webServer.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleWiFiScan() {
  StaticJsonDocument<2048> doc;
  JsonArray networks = doc.createNestedArray("networks");
  
  int numNetworks = WiFi.scanNetworks();
  
  for (int i = 0; i < numNetworks; i++) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Encrypted";
  }
  
  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleWiFiConnect() {
  if (!webServer.hasArg("ssid") || !webServer.hasArg("password")) {
    webServer.send(400, "application/json", "{\"error\":\"Missing SSID or password\"}");
    return;
  }
  
  String ssid = webServer.arg("ssid");
  String password = webServer.arg("password");
  
  // Save credentials
  strlcpy(config.ssid, ssid.c_str(), sizeof(config.ssid));
  strlcpy(config.password, password.c_str(), sizeof(config.password));
  saveConfiguration();
  
  // Try to connect
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.password);
  
  // Wait for connection
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(100);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    configMode = false;
    webServer.send(200, "application/json", "{\"status\":\"connected\",\"ip\":\"" + WiFi.localIP().toString() + "\"}");
  } else {
    webServer.send(400, "application/json", "{\"error\":\"Connection failed\"}");
  }
}

void handleGetAudioLevel() {
  StaticJsonDocument<256> doc;
  doc["level"] = audioLevel;
  doc["raw_level"] = map(audioLevel, 0, 4095, 0, 100);
  doc["sensitivity"] = config.sensitivity;
  
  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}

void handleFactoryReset() {
  factoryReset(); // This will restart the device
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("WebSocket client %u disconnected\n", num);
      break;
      
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("WebSocket client %u connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        
        // Send initial status
        StaticJsonDocument<512> doc;
        doc["type"] = "welcome";
        doc["message"] = "Connected to LED Sign";
        doc["version"] = "1.0";
        
        String message;
        serializeJson(doc, message);
        webSocket.sendTXT(num, message);
      }
      break;
      
    case WStype_TEXT:
      {
        Serial.printf("WebSocket received: %s\n", payload);
        
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          handleWebSocketMessage(num, doc);
        }
      }
      break;
      
    default:
      break;
  }
}

void handleWebSocketMessage(uint8_t clientNum, JsonDocument& doc) {
  String command = doc["command"];
  
  if (command == "set_power") {
    systemOn = doc["value"];
    if (!systemOn) {
      FastLED.clear();
      FastLED.show();
    }
  }
  else if (command == "set_mode") {
    int newMode = doc["value"];
    if (newMode >= 0 && newMode < MODE_COUNT) {
      currentMode = newMode;
      config.mode = currentMode;
      saveConfiguration();
    }
  }
  else if (command == "set_brightness") {
    config.brightness = constrain(doc["value"], 0, 100);
    saveConfiguration();
  }
  else if (command == "set_speed") {
    config.speed = constrain(doc["value"], 0, 100);
    saveConfiguration();
  }
  else if (command == "set_color") {
    JsonArray color = doc["value"];
    config.currentColor = CRGB(color[0], color[1], color[2]);
    saveConfiguration();
  }
  else if (command == "set_sensitivity") {
    config.sensitivity = constrain(doc["value"], 0, 100);
    saveConfiguration();
  }
  
  // Send acknowledgment
  StaticJsonDocument<256> response;
  response["type"] = "command_result";
  response["command"] = command;
  response["status"] = "ok";
  
  String message;
  serializeJson(response, message);
  webSocket.sendTXT(clientNum, message);
}

// HTML Content for setup mode (embedded)
String getSetupHTML() {
  return R"(
<!DOCTYPE html>
<html>
<head>
    <title>LED Sign Setup</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 40px; background: #f0f0f0; }
        .container { max-width: 500px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .form-group { margin-bottom: 20px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input, select { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
        button { background: #007bff; color: white; padding: 12px 24px; border: none; border-radius: 5px; cursor: pointer; width: 100%; font-size: 16px; }
        button:hover { background: #0056b3; }
        .status { padding: 10px; margin: 10px 0; border-radius: 5px; }
        .success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .network-item { padding: 10px; border: 1px solid #ddd; margin: 5px 0; border-radius: 5px; cursor: pointer; }
        .network-item:hover { background: #f8f9fa; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌈 LED Sign Setup</h1>
        <p>Configure your LED sign's WiFi connection to get started.</p>
        
        <div id="status"></div>
        
        <div class="form-group">
            <label>Available Networks:</label>
            <button onclick="scanNetworks()">Scan for Networks</button>
            <div id="networks"></div>
        </div>
        
        <form onsubmit="connectWiFi(event)">
            <div class="form-group">
                <label for="ssid">Network Name (SSID):</label>
                <input type="text" id="ssid" required>
            </div>
            <div class="form-group">
                <label for="password">Password:</label>
                <input type="password" id="password">
            </div>
            <button type="submit">Connect to WiFi</button>
        </form>
        
        <p style="text-align: center; margin-top: 30px; color: #666;">
            Once connected, you can access the full interface at the device's IP address.
        </p>
    </div>
    
    <script>
        function scanNetworks() {
            document.getElementById('status').innerHTML = '<div class="status">Scanning for networks...</div>';
            
            fetch('/api/wifi/scan')
                .then(response => response.json())
                .then(data => {
                    const networksDiv = document.getElementById('networks');
                    networksDiv.innerHTML = '';
                    
                    data.networks.forEach(network => {
                        const div = document.createElement('div');
                        div.className = 'network-item';
                        div.innerHTML = `<strong>${network.ssid}</strong> (${network.rssi} dBm) - ${network.encryption}`;
                        div.onclick = () => document.getElementById('ssid').value = network.ssid;
                        networksDiv.appendChild(div);
                    });
                    
                    document.getElementById('status').innerHTML = '<div class="status success">Found ' + data.networks.length + ' networks</div>';
                })
                .catch(error => {
                    document.getElementById('status').innerHTML = '<div class="status error">Error scanning networks: ' + error.message + '</div>';
                });
        }
        
        function connectWiFi(event) {
            event.preventDefault();
            
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;
            
            document.getElementById('status').innerHTML = '<div class="status">Connecting to ' + ssid + '...</div>';
            
            const formData = new FormData();
            formData.append('ssid', ssid);
            formData.append('password', password);
            
            fetch('/api/wifi/connect', {
                method: 'POST',
                body: formData
            })
            .then(response => response.json())
            .then(data => {
                if (data.status === 'connected') {
                    document.getElementById('status').innerHTML = '<div class="status success">Connected successfully! IP: ' + data.ip + '<br>You can now access the LED sign at <a href="http://' + data.ip + '">http://' + data.ip + '</a></div>';
                } else {
                    document.getElementById('status').innerHTML = '<div class="status error">Connection failed: ' + data.error + '</div>';
                }
            })
            .catch(error => {
                document.getElementById('status').innerHTML = '<div class="status error">Error: ' + error.message + '</div>';
            });
        }
        
        // Auto-scan on load
        window.onload = scanNetworks;
    </script>
</body>
</html>
)";
}

// Basic main HTML (will be replaced by full interface files)
String getMainHTML() {
  return R"(
<!DOCTYPE html>
<html>
<head>
    <title>LED Sign Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 0; padding: 20px; background: #1a1a1a; color: white; }
        .container { max-width: 800px; margin: 0 auto; }
        h1 { text-align: center; color: #00ff88; }
        .status { background: #333; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .controls { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; }
        .control-group { background: #333; padding: 20px; border-radius: 10px; }
        .control-group h3 { margin-top: 0; color: #00ff88; }
        button { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; margin: 5px; }
        button:hover { background: #0056b3; }
        input[type="range"] { width: 100%; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌈 LED Sign Control</h1>
        
        <div class="status" id="status">
            <strong>Status:</strong> <span id="status-text">Loading...</span><br>
            <strong>Mode:</strong> <span id="mode-text">-</span><br>
            <strong>Audio Level:</strong> <span id="audio-text">0</span>
        </div>
        
        <div class="controls">
            <div class="control-group">
                <h3>Power & Mode</h3>
                <button onclick="togglePower()">Toggle Power</button>
                <button onclick="nextMode()">Next Mode</button>
            </div>
            
            <div class="control-group">
                <h3>Brightness</h3>
                <input type="range" id="brightness" min="0" max="100" value="80" onchange="setBrightness(this.value)">
                <span id="brightness-value">80%</span>
            </div>
            
            <div class="control-group">
                <h3>Speed</h3>
                <input type="range" id="speed" min="0" max="100" value="50" onchange="setSpeed(this.value)">
                <span id="speed-value">50%</span>
            </div>
        </div>
        
        <p style="text-align: center; margin-top: 30px; opacity: 0.7;">
            For full interface, upload the complete web files to SPIFFS.
        </p>
    </div>
    
    <script>
        const ws = new WebSocket('ws://' + window.location.hostname + ':81');
        
        ws.onmessage = function(event) {
            const data = JSON.parse(event.data);
            if (data.type === 'status_update') {
                document.getElementById('status-text').textContent = data.power ? 'ON' : 'OFF';
                document.getElementById('mode-text').textContent = data.mode_name;
                document.getElementById('audio-text').textContent = data.audio_level;
            }
        };
        
        function togglePower() {
            ws.send(JSON.stringify({command: 'set_power', value: !getCurrentPower()}));
        }
        
        function nextMode() {
            fetch('/api/mode', {method: 'POST', body: 'mode=' + ((getCurrentMode() + 1) % 12), headers: {'Content-Type': 'application/x-www-form-urlencoded'}});
        }
        
        function setBrightness(value) {
            ws.send(JSON.stringify({command: 'set_brightness', value: parseInt(value)}));
            document.getElementById('brightness-value').textContent = value + '%';
        }
        
        function setSpeed(value) {
            ws.send(JSON.stringify({command: 'set_speed', value: parseInt(value)}));
            document.getElementById('speed-value').textContent = value + '%';
        }
        
        function getCurrentPower() {
            return document.getElementById('status-text').textContent === 'ON';
        }
        
        function getCurrentMode() {
            return 0; // Simplified for basic interface
        }
    </script>
</body>
</html>
)";
} 