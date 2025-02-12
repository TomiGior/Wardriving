#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>  // Correct LCD library for ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN 15   // SD Card Chip Select (D8 = GPIO15)
#define SDA_PIN 4      // I2C SDA (D2 = GPIO4)
#define SCL_PIN 5      // I2C SCL (D1 = GPIO5)
#define MAX_CLIENTS 4
#define FLAG_FILE "/flag.txt"
#define WINNERS_FILE "/winners.txt"

LiquidCrystal_PCF8574 lcd(0x27);

const char *ssid = "Wardriving_2024";
String passwords[] = {"wifi2024", "signal99", "router99", "access99", "network99",
                      "securityX", "wardriveX", "antennaX9", "protocolX", "SSIDpassX"};
int passwordIndex = 0;
int passwordCount = 10;

ESP8266WebServer server(80);

// Function Declarations
void scanI2CDevices();
bool checkFirstBootFlag();
void saveFirstBootFlag();
int readPasswordIndexFromSD();
void savePasswordIndexToSD(int index);
void displayPassword(String password);
void handleHome();
void handleSubmit();
void rotatePassword();
void saveWinner(String name);
String loadWinners();

void setup() {
  Serial.begin(115200);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("Running I2C Scanner...");
  scanI2CDevices();

  Serial.println("Initializing LCD...");
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD Ready!");
  Serial.println("LCD Initialized Successfully!");

  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    lcd.setCursor(0, 0);
    lcd.print("SD ERROR!");
    return;
  }
  Serial.println("SD card initialized successfully.");

  bool isFirstBoot = checkFirstBootFlag();
  String currentPassword;

  if (isFirstBoot) {
    currentPassword = "wardrivingAdmin";
    saveFirstBootFlag();
  } else {
    passwordIndex = readPasswordIndexFromSD();
    if (passwordIndex < 0 || passwordIndex >= passwordCount) {
      passwordIndex = 0;
    }
    currentPassword = passwords[passwordIndex];
  }

  displayPassword(currentPassword);

  Serial.print("Starting Access Point with password: ");
  Serial.println(currentPassword);

  bool success = WiFi.softAP(ssid, currentPassword.c_str(), 6, false, MAX_CLIENTS);

  if (success) {
    Serial.println("Access Point started successfully!");
  } else {
    Serial.println("Failed to start Access Point!");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/home", HTTP_GET, handleHome);
  server.on("/submit", HTTP_POST, handleSubmit);

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}

// ✅ I2C Scanner
void scanI2CDevices() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning I2C...");
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      Serial.println(address, HEX);
      nDevices++;
    }
  }

  if (nDevices == 0) Serial.println("No I2C devices found.");
}

// ✅ Check First Boot Flag
bool checkFirstBootFlag() {
  File file = SD.open(FLAG_FILE);
  if (!file) {
    Serial.println("Flag file not found, assuming first boot.");
    return true;
  }
  
  char flag = file.read();
  file.close();
  return (flag == '0');
}

// ✅ Save First Boot Flag
void saveFirstBootFlag() {
  File file = SD.open(FLAG_FILE, FILE_WRITE);
  if (file) {
    file.print("1");
    file.close();
    Serial.println("First boot flag saved.");
  } else {
    Serial.println("Failed to write flag file.");
  }
}

// ✅ Read Password Index from SD
int readPasswordIndexFromSD() {
  File file = SD.open("/password_index.txt");
  if (!file) {
    Serial.println("No previous password index found, defaulting to 0.");
    return 0;
  }

  int index = file.parseInt();
  file.close();
  return index;
}

// ✅ Save Password Index to SD
void savePasswordIndexToSD(int index) {
  File file = SD.open("/password_index.txt", FILE_WRITE);
  if (file) {
    file.print(index);
    file.close();
    Serial.println("Password index saved to SD.");
  } else {
    Serial.println("Failed to write password index.");
  }
}

// ✅ Display Password on LCD
void displayPassword(String password) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Password:");
  lcd.setCursor(0, 1);
  lcd.print(password);
}

// ✅ Save Winner to SD (APPENDS without Overwriting)
void saveWinner(String name) {
  File file = SD.open(WINNERS_FILE, FILE_WRITE);
  if (file) {
    file.seek(file.size()); // Move to end of file to append
    file.println(name);
    file.close();
    Serial.println("Winner saved to SD: " + name);
  } else {
    Serial.println("Failed to write to winners file.");
  }
}

// ✅ Load Winners from SD
String loadWinners() {
  File file = SD.open(WINNERS_FILE);
  if (!file) {
    Serial.println("No winners found.");
    return "No winners yet.";
  }

  String winners = "";
  while (file.available()) {
    winners += file.readStringUntil('\n');
    winners += "<br>"; // HTML line break
  }
  file.close();
  return winners;
}

// ✅ Web Page Handling (Home Page)
void handleHome() {
  String html = "<html><body>"
                "<h1>Welcome to the Scoreboard!</h1>"
                "<form action='/submit' method='POST'>"
                "<label for='name'>Enter your name or nickname:</label><br>"
                "<input type='text' id='name' name='name'><br><br>"
                "<input type='submit' value='Submit'>"
                "</form>"
                "<h2>Previous Winners:</h2>"
                "<p>" + loadWinners() + "</p>"
                "</body></html>";

  server.send(200, "text/html", html);
}

// ✅ Web Page Handling (Form Submission)
void handleSubmit() {
  if (server.hasArg("name")) {
    String winnerName = server.arg("name");
    saveWinner(winnerName);
    rotatePassword();
  }
  server.sendHeader("Location", "/home");
  server.send(303);
}

// ✅ Rotate Password on Name Submission
void rotatePassword() {
  passwordIndex = (passwordIndex + 1) % passwordCount;
  String newPassword = passwords[passwordIndex];

  savePasswordIndexToSD(passwordIndex);

  displayPassword(newPassword);
  delay(500);

  Serial.println("Restarting WiFi Access Point with new password...");
  WiFi.softAPdisconnect(true);
  delay(1000);
  WiFi.softAP(ssid, newPassword.c_str(), 6, false, MAX_CLIENTS);

  Serial.print("New Access Point password: ");
  Serial.println(newPassword);
}
