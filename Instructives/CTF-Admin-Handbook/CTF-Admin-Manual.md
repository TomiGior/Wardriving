## Explanation of the project:

We want to create a Wi-Fi CTF game that is:

- Easy to set-up & maintain.
- Low-cost.
- Portable & low power consumption.
- Autorotates the password to ensure that no one snitches the password to it.

After some research, I selected the ESP8266 and ESP32 dev boards to achieve this.

### Why? Well…

… The ESP8266 is basically a Wi-Fi-capable development board like the classic Arduino. You can even program and flash the code onto it on the Arduino IDE itself.

The ESP8266 has full AP mode support to create a WLAN network with WPA2 security protocol enabled, and also can handle a tiny HTTP server to serve a webpage that enables the user to write his/her nickname or group’s name.

The ESP8266 comes with many disadvantages against its successor the ESP32 module:

- Cannot handle multiple clients at once
- Doesn’t implement Bluetooth or BTLE technology
- Lower EEPROM values

(And because it’s fun!)

---

## Important Stuff:

Let’s proceed to the list of stuff required to build this project:

**Materials list:**

1. Two ESP8266 development boards with their original firmware installed:
    1. One will act as AP and webserver
    2. The secondary will act as Wi-Fi client that auto connects after password rotation (more on this later).
2. Two breadboards (you could use 1 but it’s more intricated and prone to confusion).
3. One SD Card “HW-125” Module.
4. One I2C LCD 16*2 display (totally optional, works for debugging/fancy-pants purposes).
5. A computer with Windows or Linux (we don’t have a Mac to test this. If you own a Mac and want to test it, please make a PR to the project 
6. A NIC capable of packet-injection and monitor mode (or Wi-Fi Pineapple)

**Software list:**

1. Arduino IDE 2.0 (you can use VSCode if you’re a hardcore maker or want to suffer for free)
    1. Board software for ESP8266.
    2. Libraries to handle I2C.
    3. Libraries to handle SD Card.
2. Wi-Fi Hacking tools:
    1. Aircrack-ng suite
    2. Hashcat 
3. Enumeration / reconnaissance tools:
    1. NMAP
    2. FFUF

> ⚠️ **Warning:**  WWindows users must install Linux apps using WSL or a Virtual Machine.

> ⚠️ **Warning:**  WiFi-Pineapple can’t install hashcat natively. Use your computer for cracking.


---

## Preparing the hardware:

Before coding, we-re going to see the schematics for our project to get everything wired before uploading our code.
Please take into account that I’m using specific (but common in market) hardware, so the source-code and schematics might not work on different Arduino module versions.

This doesn’t mean that everything is lost: just look-up the documentation for those different modules to achieve the same goal, it’s just a bit rewiring and refactoring the code.

Now let’s move on to the wiring stuff:

**Wiring tables:**

1. **SD Card:**

    | Module PIN | MCU GPIO  | MCU DIGITAL No. | Schematics Color |
    |------------|----------|----------------|------------------|
    | GND        | GND      | GND            | Blue            |
    | VCC        | 3.3v / 5v | 3.3v / 5v     | Red             |
    | MISO       | GPIO 12  | D6             | Yellow          |
    | MOSI       | GPIO 13  | D7             | Green           |
    | SCK        | GPIO 14  | D5             | Orange          |
    | CS         | GPIO 15  | D8             | Ochre           |

2. **I2C LCD Screen:**
    
    | Module PIN | MCU GPIO  | MCU Digital No. | Schematics Color |
    |------------|----------|----------------|------------------|
    | GND        | GND      | GND            | Blue            |
    | VCC        | 3.3v / 5v | 3.3v / 5v     | Red             |
    | SDA        | GPIO 4   | D2             | Pink            |
    | SCL        | GPIO 5   | D1             | Black           |

Knowing the pinout, let’s proceed to the wiring schematics:

![ESP8266-Schematics](/Media/ESP8266/ESP8266-Schematics.jpg)

---

## Coding & flashing the ESP8266 CTF:

After wiring the whole project up, you must upload the code to the board.

To do so, you must clone the Wardriving repository to your machine:

```bash
git clone https://github.com/TomiGior/Wardriving.git
```

After cloning the repo, you must open the sketch using the Arduino IDE. To achieve this chose open file, and there you must select the ESP8266CTF project file stored at …/Extras/ESP8266CTF-Firmware/ESP8266-CTF/ESP8266-CTF.ino

![ESP8266-Arduino-IDE.gif](/Media/ESP8266/ESP8266-Arduino-IDE.gif)

After this, we must plug the ESP8266 NodeMCU dev board to the USB of our machine and install/set the required libraries/boards at Arduino IDE:

1. ESP8266/ESP32 Board support.
2. LiquidCrystal_PCF8574
3. Set board to Generic NodeMCU ESP8266

Install the ESP8266 board libraries:
We must set the additional board manager URLs that will allow us to download the necessary files to be able to compile and flash code to the ESP8266.

To do so, we must click on File → Preferences → Edit “Additional board manager URLs:” text-area with this:

```latex
http://arduino.esp8266.com/stable/package_esp8266com_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Then click OK and go to the boards manager. Search for the ESP8266 board and install it:

![ESP8266-Board-Install.gif](/Media/ESP8266/ESP8266-Board-Install.gif)

Then, we must install the libraries for the display. 
To achieve this, we must search for in on the library manager by typing “LiquidCrystal_PCF8574” by Matthias Hertel:

![Install_LCD_library.png](/Media/ESP8266/Install_LCD_library.png)

Again, remember I’m using specific hardware. The models of the I2C LCD Screens may vary. If your sketch fails to compile, look-up your module’s information and which library is used for them. Although most libraries made for the various manufacturers of the same kind of modules work exactly the same, some may vary in some functions name, be careful with that when refactoring the code.

After installing this library, we must plug our ESP8266 NodeMCU to our computer’s USB and select the board at the board manager’s quick menu. Ensure that the correct COM port is selected (you can plug and unplug your device to check this):

![Select-Board-8266.gif](/Media/ESP8266/Select-Board-8266.gif)

Then, we can upload the code to the ESP8266 NodeMCU Board:

![Flash8266Firmware.gif](/Media/ESP8266/Flash8266Firmware.gif)

After the firmware is flashed, we’re ready to go.

You may want to read the CTF Player Manual to be able to check if the project worked.
