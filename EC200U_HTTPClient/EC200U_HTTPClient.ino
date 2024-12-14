/*
 * Project Name: ESP32 and EC200U-CN GPRS HTTP GET Request
 * Description: This code establishes a GPRS connection using the EC200U-CN module and sends an HTTP GET 
 *              request to a specified server. It demonstrates initializing the modem, configuring APN 
 *              settings, and retrieving data from the server.
 * 
 * Hardware Connections:
 * - ESP32:
 *   - RX2 (ESP32) -> TX (EC200U-CN)
 *   - TX2 (ESP32) -> RX (EC200U-CN)
 *   - GND (ESP32) -> GND (EC200U-CN)
 *
 * - EC200U-CN Module:
 *   - Ensure a valid SIM card is inserted.
 *   - Power the module with a stable power supply as per its requirements.
 * 
 * Notes:
 * - Replace the `apn`, `user`, and `pass` variables with your network provider's APN details.
 * - Update the `server` variable with the URL you wish to access.
 * 
 * Learn More:
 * For a complete step-by-step guide on setting up the EC200U-CN module and using it with ESP32, check out 
 * my YouTube channel, **Techiesms**. Subscribe for exciting DIY tech projects and tutorials!
 * YouTube: [Techiesms](https://www.youtube.com/techiesms)
 * 
 */

#define SerialMon Serial
#define SerialAT Serial2

#define MODEM_BAUDRATE 115200

const char apn[] = "www";  // Replace with your APN
const char user[] = "";    // Replace with your APN username (if any)
const char pass[] = "";    // Replace with your APN password (if any)

const char* server = "http://vsh.pp.ua/TinyGSM/logo.txt";  // URL to access

void setup() {
  SerialMon.begin(115200);         // Serial monitor
  SerialAT.begin(MODEM_BAUDRATE);  // Serial connection to modem

  SerialMon.println("Initializing modem...");

  modem_init();  // Send initialization commands

  // Configure GPRS settings
  sendATCommand(String("AT+QICSGP=1,1,\"") + apn + "\",\"" + user + "\",\"" + pass + "\",1");
  sendATCommand("AT+QIACT=1");  // Activate PDP context

  SerialMon.println("Connected to network.");
  sendHttpGetRequest();
}

void sendHttpGetRequest() {
  SerialMon.println("Sending HTTP GET request...");

  // Set up HTTP GET request
  sendATCommand("AT+QHTTPCFG=\"contextid\",1");                            // Use PDP context 1
  sendATCommand(String("AT+QHTTPURL=") + String(strlen(server)) + ",80");  // Set URL
  SerialAT.print(server);
  SerialAT.write(0x1A);  // End of input with Ctrl+Z
  delay(1000);

  // Send HTTP GET request
  sendATCommand("AT+QHTTPGET=80");

  // Read HTTP response
  sendATCommand("AT+QHTTPREAD");

  // Disconnect HTTP session
  sendATCommand("AT+QHTTPEND");
}

void loop() {
  // Keep the loop empty for now
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

  delay(1000);  // Allow some time for the modem to respond

  while (SerialAT.available()) {
    String response = SerialAT.readString();
    SerialMon.print("Response: ");
    SerialMon.println(response);
  }
}