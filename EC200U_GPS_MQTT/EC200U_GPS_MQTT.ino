/*
 * Project Name: GPS Data Transmission over MQTT using EC200U-CN Module and ESP32
 * Description: This code integrates GPS location tracking and MQTT communication using the EC200U-CN module 
 *              and ESP32. The GPS module retrieves latitude and longitude, and the MQTT module sends the data 
 *              to a specified broker at regular intervals.
 * 
 * Hardware Connections:
 * - ESP32:
 *   - RX2 (ESP32) -> TX (EC200U-CN)
 *   - TX2 (ESP32) -> RX (EC200U-CN)
 *   - GND (ESP32) -> GND (EC200U-CN)
 * - EC200U-CN Module:
 *   - Insert a valid SIM card with GPRS capabilities.
 *   - Power the module with a stable power supply.
 * 
 * Notes:
 * - Replace the placeholders in `apn`, `mqttServer`, `mqttPort`, `mqttUser`, `mqttPassword`, and `mqttTopic` 
 *   with your network provider and MQTT broker details.
 * - The code retrieves GPS data every 5 seconds and publishes it to the MQTT topic.
 * - Ensure the module has a clear line of sight to the sky for accurate GPS data.
 * 
 * Learn More:
 * For a comprehensive guide on GPS and MQTT integration using EC200U-CN and ESP32, visit my YouTube channel, 
 * **Techiesms**. Subscribe for tutorials on IoT, embedded systems, and exciting tech projects.
 * YouTube: [Techiesms](https://www.youtube.com/techiesms)
 * 
 */

// Define serial communication
#define SerialMon Serial       // For debugging
#define SerialAT Serial2       // For EC200U-CN communication
#define MODEM_BAUDRATE 115200  // Modem communication baud rate

// MQTT Configuration
const char apn[] = "www";                                        // Replace with your APN
const char mqttServer[] = "MQTT BROKER";                     // MQTT Server
const int mqttPort = 1883;                                       // MQTT Port
const char mqttUser[] = "USERNAME";                            // MQTT Username
const char mqttPassword[] = "PASSWORD";  // MQTT Password
const char mqttTopic[] = "PUB TOPIC";            // MQTT Topic

void setup() {
  // Start Serial for debug
  SerialMon.begin(115200);
  while (!SerialMon) {}

  // Start Serial for EC200U-CN
  SerialAT.begin(MODEM_BAUDRATE);

  SerialMon.println("Initializing modem...");
  modem_init();  // Send initialization commands

  // Turn off GPS and MQTT (clean startup)
  sendATCommand("AT+QGPSEND");
  sendATCommand("AT+QMTDISC=0");
  delay(2000);

  // Configure MQTT
  initializeMQTT();

  // Initialize GPS
  SerialMon.println("Starting GPS...");
  sendATCommand("AT+QGPS=1");
  delay(10000);  // Wait for GPS to initialize
}

void loop() {
  // Query GPS location
  SerialAT.println("AT+QGPSLOC=2");
  delay(2000);  // Wait for response

  String response = readATResponse();
  response = response.substring(2);
  if (response.startsWith("+QGPSLOC:")) {
    String lat, lon;
    if (parseGPSData(response, lat, lon)) {
      // Format location data
      String locationData = "Latitude: " + lat + ", Longitude: " + lon;
      SerialMon.println("Location Data: " + locationData);

      // Send data to MQTT
      publishToMQTT(locationData);
    } else {
      SerialMon.println("Failed to parse GPS data.");
    }
  } else {
    SerialMon.println("No valid GPS data available yet.");
  }

  delay(5000);  // Publish every 5 seconds
}

void initializeMQTT() {
  // Configure GPRS
  sendATCommand(String("AT+QICSGP=1,1,\"") + apn + "\",\"\",\"\",1");
  sendATCommand("AT+QIACT=1");

  // Connect to MQTT broker
  sendATCommand(String("AT+QMTOPEN=0,\"") + mqttServer + "\"," + mqttPort);
  sendATCommand(String("AT+QMTCONN=0,\"123\",\"") + mqttUser + "\",\"" + mqttPassword + "\"");
}

void publishToMQTT(const String &data) {
  SerialMon.println("Publishing to MQTT...");
  sendATCommand(String("AT+QMTPUB=0,0,0,0,\"") + mqttTopic + "\"");
  SerialAT.print(data);
  SerialAT.write(0x1A);  // End of input with Ctrl+Z
}

String readATResponse() {
  String response = "";
  long timeout = millis() + 3000;  // 3-second timeout for reading the response

  while (millis() < timeout) {
    while (SerialAT.available()) {
      char c = SerialAT.read();
      response += c;
    }
    if (response.length() > 0) break;  // Stop reading if response is received
  }

  SerialMon.println("Raw Response: " + response);
  return response;
}

bool parseGPSData(const String &response, String &latitude, String &longitude) {
  // Example response: +QGPSLOC: 105824.00,22.5726,N,88.3639,E,10.5,0.0,0.0,230394,03
  int firstComma = response.indexOf(',');                   // Find the first comma
  int secondComma = response.indexOf(',', firstComma + 1);  // Find the second comma
  int thirdComma = response.indexOf(',', secondComma + 1);  // Find the third comma

  if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
    latitude = response.substring(firstComma + 1, secondComma);
    longitude = response.substring(secondComma + 1, thirdComma);
    return true;
  }
  return false;
}

void modem_init() {

  sendATCommand("AT");         // Basic AT command to check communication
  sendATCommand("ATE0");       // Disable echo for cleaner responses
  sendATCommand("AT+CPIN?");   // Check SIM status
  sendATCommand("AT+CSQ");     // Check signal quality
  sendATCommand("AT+CREG?");   // Check network registration status
  sendATCommand("AT+CGATT?");  // Check if GPRS is attached
}

void sendATCommand(const String &command) {
  SerialAT.println(command);
  delay(500);  // Short delay for response

  // Read and display the response
  while (SerialAT.available()) {
    String response = SerialAT.readString();
    SerialMon.println(response);
  }
}
