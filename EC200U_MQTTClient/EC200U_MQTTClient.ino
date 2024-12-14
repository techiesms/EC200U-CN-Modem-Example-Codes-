/*
 * Project: MQTT Communication using EC200U-CN Module
 * Description: This project demonstrates the use of the EC200U-CN module for MQTT communication. 
 *              The code establishes an MQTT connection, publishes data at regular intervals, 
 *              and subscribes to a topic to receive messages.
 *
 * Features:
 * - Publishes incrementing counter data to a specified MQTT topic.
 * - Listens for incoming messages on a subscribed MQTT topic.
 *
 * Requirements:
 * - Ensure correct APN settings for your network provider.
 * - Replace MQTT server, username, password, and topics with your broker's configuration.
 *
 * Hardware Connections:
 * - ESP32:
 *   - RX2 (ESP32) -> TX (EC200U-CN)
 *   - TX2 (ESP32) -> RX (EC200U-CN)
 *   - GND (ESP32) -> GND (EC200U-CN)
 
 * Author: Sachin Soni
 * YouTube: Check out tech tutorials and projects at **techiesms**: https://www.youtube.com/techiesms
 */


#define SerialMon Serial
#define SerialAT Serial2

#define MODEM_BAUDRATE 115200

const char apn[] = "www";  // Replace with your APN
const char user[] = "";    // Replace with APN username (if any)
const char pass[] = "";    // Replace with APN password (if any)

const char mqttServer[] = "MQTT SERVER";                     // MQTT Server
const int mqttPort = 1883;                                       // MQTT Port
const char mqttUser[] = "USERNAME";                            // MQTT Username
const char mqttPassword[] = "PASSWORD";  // MQTT Password
const char mqttPublishTopic[] = "PUB TOPIC";       // Topic to publish
const char mqttSubscribeTopic[] = "SUB TOPIC";    // Topic to subscribe

unsigned long lastPublishTime = 0;           // Tracks the last publish time
const unsigned long publishInterval = 5000;  // Publish interval (5 seconds)

void setup() {
  SerialMon.begin(115200);
  SerialAT.begin(MODEM_BAUDRATE);

  SerialMon.println("Initializing modem...");

  modem_init();  // Send initialization commands

  // Ensure no previous MQTT connections exist
  SerialMon.println("Disconnecting previous MQTT sessions...");
  sendATCommand("AT+QMTDISC=0");                                                                // Disconnect MQTT session (if any)
  sendATCommand("AT+QMTOPEN=0,\"" + String(mqttServer) + "\"," + String(mqttPort));             // Open MQTT connection
  checkResponse("AT+QMTCONN=0,\"123\",\"" + String(mqttUser) + "\",\"" + mqttPassword + "\"");  // Connect to MQTT broker

  SerialMon.println("Subscribing to MQTT topic...");
  checkResponse(String("AT+QMTSUB=0,1,\"") + mqttSubscribeTopic + "\",0");  // Subscribe to topic
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to publish data
  if (currentMillis - lastPublishTime >= publishInterval) {
    publishData();
    lastPublishTime = currentMillis;
  }

  // Continuously check for incoming MQTT messages
  checkForMQTTMessages();
}

void publishData() {
  static int counter = 0;              // Data to publish
  String payload = String(counter++);  // Increment data to publish

  SerialMon.println("Publishing data: " + payload);
  sendATCommand(String("AT+QMTPUB=0,0,0,0,\"") + mqttPublishTopic + "\"");
  SerialAT.print(payload);
  SerialAT.write(0x1A);  // End of input with Ctrl+Z
}

void checkForMQTTMessages() {
  while (SerialAT.available()) {
    String response = SerialAT.readString();
    //SerialMon.println("Received MQTT Message: " + response);

    if (response.indexOf(mqttSubscribeTopic) != -1) {
      SerialMon.println("Message received on subscribed topic!");
      int startIndex = response.indexOf(",") + 1;
      String message = response.substring(startIndex);
      SerialMon.println("Message content: " + message);
    }
  }
}
void modem_init() {

  sendATCommand("AT");         // Basic AT command to check communication
  sendATCommand("ATE0");       // Disable echo for cleaner responses
  sendATCommand("AT+CPIN?");   // Check SIM status
  sendATCommand("AT+CSQ");     // Check signal quality
  sendATCommand("AT+CREG?");   // Check network registration status
  sendATCommand("AT+CGATT?");  // Check if GPRS is attached
}

void sendATCommand(const String& command) {
   SerialMon.print("Sending: ");
  SerialMon.println(command);
  SerialAT.println(command);
  delay(1000);  // Minimal delay to allow command processing

  while (SerialAT.available()) {
    String response = SerialAT.readString();
    SerialMon.println("Response: " + response);
  }
}

void checkResponse(const String& command) {
   SerialMon.print("Sending: ");
  SerialMon.println(command);
  SerialAT.println(command);
  delay(1000);  // Minimal delay to allow command processing

  while (SerialAT.available()) {
    String response = SerialAT.readString();
    SerialMon.print("Response: ");
    SerialMon.println(response);
    if (response.indexOf("ERROR") != -1) {
      SerialMon.println("Error in response!");
      while (true)
        ;  // Halt on error
    }
  }
}
