/*
 * Project: Sending SMS using EC200U-CN Module
 * Description: This project demonstrates how to use the EC200U-CN module to send an SMS message 
 *              to a specified phone number. The message is sent via the AT command set to control 
 *              the module and communicate with the cellular network.
 *
 * Features:
 * - Initializes the EC200U-CN module.
 * - Sends a test SMS message to a specified phone number.
 *
 * Requirements:
 * - Replace the phone number with the recipient's number.
 * - Ensure that your SIM card is inserted and properly activated.
 *
 * Hardware Connections:
 * - ESP32:
 *   - RX2 (ESP32) -> TX (EC200U-CN)
 *   - TX2 (ESP32) -> RX (EC200U-CN)
 *   - GND (ESP32) -> GND (EC200U-CN)
 *
 * Author: Sachin Soni
 * YouTube: For more tech tutorials and projects, visit **techiesms**: https://www.youtube.com/techiesms
 */

// Define serial communication
#define SerialMon Serial       // For debugging
#define SerialAT Serial2       // For EC200U-CN communication
#define MODEM_BAUDRATE 115200  // Modem communication baud rate

const char phoneNumber[] = "+91XXXXXXXXXX";                             // Replace with the recipient's phone number
const char message[] = "Hello, this is a test message from EC200U!";  // SMS message

void setup() {
  // Start Serial for debug
  SerialMon.begin(115200);
  while (!SerialMon) {}

  // Start Serial for EC200U-CN
  SerialAT.begin(MODEM_BAUDRATE);

  SerialMon.println("Initializing modem...");

  modem_init();  // Send initialization commands

  // Send SMS
  SerialMon.println("Sending SMS...");
  sendATCommand("AT+CMGF=1");                                // Set SMS text mode
  sendATCommand(String("AT+CMGS=\"") + phoneNumber + "\"");  // Set recipient
  SerialAT.print(message);
  SerialAT.write(0x1A);  // End of message with Ctrl+Z

  SerialMon.println("SMS sent!");
}

void loop() {
  // Nothing to do in the loop
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
  SerialMon.print("Sending: ");
  SerialMon.println(command);
  SerialAT.println(command);
  delay(1000);  // Wait for response

  // Read and display the response
  while (SerialAT.available()) {
    String response = SerialAT.readString();
    SerialMon.print("Response: ");
    SerialMon.println(response);
  }
}
