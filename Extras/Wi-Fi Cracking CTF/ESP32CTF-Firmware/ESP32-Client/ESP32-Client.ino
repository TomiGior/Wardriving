#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>

const char* ssid = "Wardriving_2024";
const char* firstPassword = "wifi2024";  // First password is always this

const char* esp8266_ip = "192.168.4.1";  // Change this to your ESP8266's IP
const int udpPort = 4210;

WiFiUDP udp;
String lastPassword = firstPassword;  // Start with the hardcoded first password

void sendMockTraffic();
void sendUDPMessage();
void sendHTTPRequest();
void checkWiFiStatus();
void attemptReconnect();

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Ready");

  // Initial connection with the hardcoded first password
  attemptReconnect();
}

void loop() {
  // Check if a new password is received from Serial
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // Read incoming serial data

    if (input.startsWith("NEW PASSWORD: ")) { // Filter only password messages
      lastPassword = input.substring(14); // Extract the password
      lastPassword.trim(); // Remove any trailing spaces or newline characters

      Serial.print("Received New Password: ");
      Serial.println(lastPassword);

      attemptReconnect();  // Connect with the new received password
    }
  }

  // Check for deauthentication every 5 seconds
  checkWiFiStatus();

  // If connected, send mock traffic
  if (WiFi.status() == WL_CONNECTED) {
    sendMockTraffic();
    delay(3000);  // Traffic interval
  }
}

// Function to check WiFi connection status and detect deauthentication
void checkWiFiStatus() {
  static unsigned long lastCheckTime = 0;
  unsigned long currentTime = millis();

  // Check every 5 seconds
  if (currentTime - lastCheckTime > 5000) {
    lastCheckTime = currentTime;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WARNING] Deauthentication detected! Attempting reconnect...");
      attemptReconnect();
    }
  }
}

// Function to attempt reconnection using the last known password
void attemptReconnect() {
  Serial.println("Attempting to connect...");

  WiFi.disconnect(true);
  WiFi.begin(ssid, lastPassword.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nReconnection failed. Will retry...");
  }
}

// Function to send mock network traffic
void sendMockTraffic() {
  Serial.println("Sending mock network traffic...");

  // Send UDP Message
  sendUDPMessage();
  
  // Send HTTP Request
  sendHTTPRequest();
}

// Function to send UDP packet
void sendUDPMessage() {
  Serial.println("Sending UDP packet...");
  
  udp.beginPacket(esp8266_ip, udpPort);
  udp.print("Hello ESP8266 from ESP32!");
  udp.endPacket();

  Serial.println("UDP packet sent.");
}

// Function to send HTTP GET request
void sendHTTPRequest() {
  Serial.println("Sending HTTP GET request...");

  HTTPClient http;
  String url = "http://" + String(esp8266_ip) + "/ping"; // Change endpoint if needed

  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.println("Failed to send HTTP request.");
  }
  http.end();
}
