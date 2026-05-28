# LAN-Controlled Automatic Plant Watering System 🌱🔌

A reliable, offline-first smart agricultural system designed to monitor and automatically water plants. Built around the ESP32 microcontroller, this project hosts a local web server, allowing users to control the system and view real-time data strictly over a Local Area Network (LAN) without requiring an active internet connection.

## 🚀 Features
* **Offline Local Control:** Hosts an embedded HTML/JS web interface directly on the ESP32, accessible via local IP address.
* **No Internet Required:** Operates 100% locally on your home Wi-Fi network (LAN), ensuring privacy and high reliability even during internet outages.
* **Real-time Monitoring:** Displays live sensor data (e.g., soil moisture levels) on the web dashboard.
* **Dual Operation Modes:** * *Automatic Mode:* Executes predefined C++ logic to water plants automatically when soil moisture falls below a certain threshold.
  * *Manual Mode:* Allows users to manually trigger the water pump from the web interface.

## 🛠️ Tech Stack
**Hardware:**
* ESP32 Microcontroller
* Soil Moisture Sensor 
* Relay Module & Water Pump

**Firmware & Software:**
* **Firmware:** Arduino C++
* **Web Interface:** HTML, JavaScript (Hosted on ESP32 SPIFFS/PROGMEM)
* **Networking:** Local Web Server, ESP32 Wi-Fi (Station/AP mode)

## 🏗️ System Architecture
1. **Data Collection:** Environmental sensors continuously read soil conditions and send signals to the ESP32.
2. **Local Hosting:** The ESP32 acts as a local web server on the network, serving the HTML and JavaScript files to any connected client (browser/phone).
3. **Execution:** When the user sends a command via the web dashboard, the ESP32 processes the HTTP request locally and activates the relay for the water pump.

## ⚙️ Setup & Installation
1. Open the source code in **Arduino IDE** or **Visual Studio Code (PlatformIO)**.
2. Update the Wi-Fi credentials (`SSID` and `PASSWORD`) in the configuration to match your local network.
3. Flash the firmware (and upload the HTML/JS files to the SPIFFS/LittleFS if they are separated) to the ESP32.
4. Open the Serial Monitor to find the ESP32's local IP address (e.g., `192.168.1.x`).
5. Type that IP address into any web browser connected to the same Wi-Fi network to access the control dashboard.
