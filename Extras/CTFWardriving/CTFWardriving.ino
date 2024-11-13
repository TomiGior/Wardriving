//Created by S731N for entertainment and educational purposes
//S731N, the creator of this script, allows the usage of this sourcecode in ethical demonstrations.
//The creator of this software is not responsible for any damage caused to your board, as this comes with no warranty and was tested only on two boards.
#include <EEPROM.h>
#include <WiFi.h> // Use #include <ESP8266WiFi.h> for ESP8266
#include <WebServer.h>
#include <MD5Builder.h>

#define EEPROM_SIZE 512
#define EEPROM_WRITE_THRESHOLD 100
#define MAX_UNPRINTABLE_COUNT 5

const char *ssid = "Wardriving 2024";
String passwords[] = {"wifi", "signal", "router", "access", "network", "security", "wardrive", "antenna", "protocol", "SSID"};
int passwordIndex = 0;
int passwordCount = 10;
int eepromWriteCounter = 0;

WebServer server(80);

String scoreboard[10]; // Scoreboard to store 10 names max
int scoreboardStart = 0; // Start index for circular buffer
int scoreboardSize = 0; // Current size of the scoreboard

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // Load data from EEPROM
  loadScoreboard();
  loadPasswordIndex();

  // Set up WiFi Access Point
  WiFi.softAP(ssid, passwords[passwordIndex].c_str());
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define the web server route for /home
  server.on("/home", HTTP_GET, handleHome);
  server.on("/submit", HTTP_POST, handleSubmit);

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}

void handleHome() {
  String html = "<html><body>"
                "<h1>Welcome to the Scoreboard!</h1>"
                "<form action='/submit' method='POST'>"
                "<label for='name'>Enter your name or nickname:</label><br>"
                "<input type='text' id='name' name='name'><br><br>"
                "<input type='submit' value='Submit'>"
                "</form>"
                "<h2>Scoreboard:</h2>";

  // Display current scoreboard
  for (int i = 0; i < scoreboardSize; i++) {
    int index = (scoreboardStart + i) % 10;
    if (scoreboard[index].length() > 0) {
      html += String(i + 1) + ". " + scoreboard[index] + "<br>";
    }
  }

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  if (server.hasArg("name")) {
    String name = server.arg("name");
    name.trim();
    name.toLowerCase();
    addToScoreboard(name);
    saveScoreboard();
    rotatePassword();
  }
  server.sendHeader("Location", "/home");
  server.send(303);

  // Use a non-blocking mechanism instead of delay
  unsigned long startTime = millis();
  while (millis() - startTime < 500) {
    server.handleClient();
  }
}

void addToScoreboard(String name) {
  // Check for duplicate names before adding
  name.trim();
  name.toLowerCase();
  for (int i = 0; i < scoreboardSize; i++) {
    int index = (scoreboardStart + i) % 10;
    if (scoreboard[index] == name) {
      return; // Name already exists, do not add
    }
  }

  // Add name to the scoreboard using a circular buffer
  int newIndex = (scoreboardStart + scoreboardSize) % 10;
  if (scoreboardSize < 10) {
    scoreboard[newIndex] = name;
    scoreboardSize++;
  } else {
    scoreboard[scoreboardStart] = name;
    scoreboardStart = (scoreboardStart + 1) % 10;
  }
}

void saveScoreboard() {
  // Calculate the checksum of the current scoreboard
  MD5Builder md5;
  md5.begin();
  for (int i = 0; i < scoreboardSize; i++) {
    int index = (scoreboardStart + i) % 10;
    md5.add(scoreboard[index]);
  }
  md5.calculate();
  String currentChecksum = md5.toString();

  // Load the stored checksum from EEPROM
  String storedChecksum = "";
  int addr = 400;
  for (int i = 0; i < 32; i++) {
    storedChecksum += char(EEPROM.read(addr++));
  }

  // If checksums do not match, update EEPROM
  if (currentChecksum != storedChecksum) {
    addr = 0;
    for (int i = 0; i < scoreboardSize; i++) {
      int index = (scoreboardStart + i) % 10;
      for (int j = 0; j < scoreboard[index].length(); j++) {
        if (EEPROM.read(addr) != scoreboard[index][j]) {
          EEPROM.write(addr, scoreboard[index][j]);
        }
        addr++;
      }
      if (EEPROM.read(addr) != '\0') {
        EEPROM.write(addr, '\0'); // Add null terminator
      }
      addr++;
    }
    EEPROM.commit();

    // Store the new checksum
    addr = 400;
    for (int i = 0; i < 32; i++) {
      if (EEPROM.read(addr) != currentChecksum[i]) {
        EEPROM.write(addr, currentChecksum[i]);
      }
      addr++;
    }
    EEPROM.commit();
  }
}

void loadScoreboard() {
  // Load scoreboard from EEPROM
  int addr = 0;
  scoreboardSize = 0;
  scoreboardStart = 0;
  for (int i = 0; i < 10; i++) {
    String name = "";
    char c;
    int unprintableCount = 0;
    while ((c = EEPROM.read(addr++)) != '\0' && name.length() < 31 && addr < EEPROM_SIZE) {
      // Validate that the character is printable to avoid incorrect data
      if (isPrintable(c)) {
        name += c;
        unprintableCount = 0; // Reset unprintable count when valid character is read
      } else {
        unprintableCount++;
        if (unprintableCount >= MAX_UNPRINTABLE_COUNT) {
          break; // Break if too many unprintable characters are encountered
        }
      }
    }
    if (name.length() > 0) {
      scoreboard[scoreboardSize++] = name;
    }
  }
}

void rotatePassword() {
  int oldPasswordIndex = passwordIndex;
  passwordIndex = (passwordIndex + 1) % passwordCount;
  String newPassword = passwords[passwordIndex];

  // Only restart WiFi AP if the new password is different from the current one
  if (passwordIndex != oldPasswordIndex) {
    // Remove used password from the list
    for (int i = passwordIndex; i < passwordCount - 1; i++) {
      passwords[i] = passwords[i + 1];
    }
    passwordCount--;

    savePasswordIndex();
    WiFi.softAP(ssid, newPassword.c_str());
    Serial.print("New Access Point password: ");
    Serial.println(newPassword);
  }
}

void savePasswordIndex() {
  if (EEPROM.read(500) != passwordIndex) {
    EEPROM.write(500, passwordIndex);
    EEPROM.commit();
  }
}

void loadPasswordIndex() {
  passwordIndex = EEPROM.read(500);
  if (passwordIndex < 0 || passwordIndex >= passwordCount) {
    passwordIndex = 0;
  }
}
