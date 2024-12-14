/*
 * Project Name: GPS Location Data with EC200U-CN Module and ESP32
 * Description: This code demonstrates how to use the EC200U-CN GSM module with ESP32 to retrieve GPS 
 *              location data. It initializes the modem, configures the GPS module, and reads and parses 
 *              location data from the module.
 * 
 * Hardware Connections:
 * - ESP32:
 *   - RX2 (ESP32) -> TX (EC200U-CN)
 *   - TX2 (ESP32) -> RX (EC200U-CN)
 *   - GND (ESP32) -> GND (EC200U-CN)
 * - EC200U-CN Module:
 *   - Ensure a valid SIM card is inserted.
 *   - Power the module with a stable power supply as per its requirements.
 * 
 * Notes:
 * - The code assumes that the EC200U-CN module has built-in GPS functionality.
 * - GPS data is queried every 5 seconds, and the location (latitude and longitude) is displayed in the Serial Monitor.
 * - Make sure the module has a clear line of sight to the sky for accurate GPS data.(Try out this code near window, in balcony or on Terrace)
 * 
 * Learn More:
 * For a detailed tutorial on GPS location tracking with the EC200U-CN module and ESP32, visit my YouTube channel, 
 * **Techiesms**. Subscribe for exciting tech projects and step-by-step guides to DIY innovations!
 * YouTube: [Techiesms](https://www.youtube.com/techiesms)
 * 
 */


#define SerialMon Serial       // Serial monitor for debug
#define SerialAT Serial2       // Serial communication with EC200U-CN
#define MODEM_BAUDRATE 115200  // Baudrate for modem communication

void setup() {
  // Initialize Serial for debug output
  SerialMon.begin(115200);
  while (!SerialMon) {}

  // Initialize Serial for EC200U-CN
  SerialAT.begin(MODEM_BAUDRATE);
  delay(1000);

  SerialMon.println("Initializing modem...");
  modem_init();  // Send initialization commands

  SerialMon.println("Initializing GPS...");

  // Turn off GPS (if previously on)
  sendATCommand("AT+QGPSEND");
  delay(2000);  // Ensure GPS is off

  // Start GPS
  sendATCommand("AT+QGPS=1");
  delay(10000);  // Wait for GPS to initialize
}

void loop() {
  // Query GPS location
  SerialAT.println("AT+QGPSLOC=2");
  delay(2000);  // Short wait for a response

  String response = readATResponse();

  if (response.startsWith("+QGPSLOC:")) {
    parseAndPrintLocation(response);
  } else {
    SerialMon.println("No valid GPS data available yet.");
  }

  delay(5000);  // Wait 5 seconds before the next query
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
  response = response.substring(2);
  SerialMon.println("Raw Response: " + response);
  return response;
}

void parseAndPrintLocation(const String &response) {
  // Example response: +QGPSLOC: 105824.00,22.5726,N,88.3639,E,10.5,0.0,0.0,230394,03
  int firstComma = response.indexOf(',');                   // Find the first comma
  int secondComma = response.indexOf(',', firstComma + 1);  // Find the second comma
  int thirdComma = response.indexOf(',', secondComma + 1);  // Find the third comma

  if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
    String latitude = response.substring(firstComma + 1, secondComma);
    String longitude = response.substring(secondComma + 1, thirdComma);

    SerialMon.println("Latitude: " + latitude);
    SerialMon.println("Longitude: " + longitude);
  } else {
    SerialMon.println("Failed to parse GPS data.");
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

void sendATCommand(const char *command) {
  SerialMon.print("Sending: ");
  SerialMon.println(command);
  SerialAT.println(command);
  delay(500);  // Short delay for response

  // Read and display the response
  while (SerialAT.available()) {
    String response = SerialAT.readString();
    SerialMon.print("Response: ");
    SerialMon.println(response);
  }
}
