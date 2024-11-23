# ESP32 UDP/WebSocket Chat

## About

A real-time chat application using ESP32, featuring both WebSocket and UDP communication protocols. The application provides a modern, responsive web interface for seamless communication between multiple clients.

## Features

- Real-time messaging using WebSocket and UDP
- Modern, responsive web interface
- Username-based chat system
- Connection status indicators
- Auto-reconnect functionality
- mDNS support for easy access

## Prerequisites

- Arduino IDE 1.8.18
- ESP32 board support
  - Install ESP32 board package in Arduino IDE via Boards Manager
  - Select appropriate ESP32 board from Tools > Board menu

### Required Libraries
1. AsyncTCP: https://github.com/me-no-dev/AsyncTCP
2. ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer
3. ArduinoJson: Install via Library Manager
4. ESP32 SPIFFS Upload Tool: https://github.com/me-no-dev/arduino-esp32fs-plugin
5. WiFi.h (included in ESP32 board package)
6. ESPmDNS.h (included in ESP32 board package)
7. SPIFFS.h (included in ESP32 board package)
8. AsyncUDP.h (included in ESP32 board package)

### Hardware
- ESP32 Development Board
- USB Cable
- Computer with Arduino IDE installed

## Installation

1. Clone this repository
2. Open `esp32_chat.ino` in Arduino IDE 1.8.18
3. Install required libraries using the Arduino Library Manager
4. Update WiFi credentials in the code:
   ```cpp
   const char* ssid = "Your_SSID";
   const char* password = "Your_Password";
   ```
5. Upload the SPIFFS data:
   - Install the [ESP32 Sketch Data Upload tool](https://github.com/me-no-dev/arduino-esp32fs-plugin)
   - Go to Tools > ESP32 Sketch Data Upload
   - Wait for the upload to complete

## Usage

1. Upload the code to your ESP32
2. Open the Serial Monitor to view the ESP32's IP address
3. Access the chat interface:
   - Using mDNS: Navigate to `http://esp32chat.local` in your web browser
   - If mDNS doesn't work, use the IP address shown in Serial Monitor (e.g., `http://192.168.1.100`)
4. Enter a username when prompted
5. Start chatting!

## Project Structure

- `esp32_chat.ino` - Main Arduino sketch
- `data/index.html` - Web interface
- Client management for both WebSocket and UDP connections
- Automatic cleanup of stale connections

## Credits

Created by Kent Carlo Amante
