#include <SPI.h>
#include <Ethernet.h>

// Setup Mac address (Hardware Address) for Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
EthernetClient client;

int ampPin = A0;  // Define ampPin analog pin A0 (Reading Solar Amps)
int voltPin = A1; // Define voltPin analog pin A1 (Reading Solar Voltage)

// Declare Values
float ampValue = 0;
float voltValue = 0;
float wattValue = 0;

// Emoncms configurations
char server[] = "emoncms.org"; // name address for emoncms.org

String apikey = "8************"; // api write key for emoncms.org
int node = 1; // if 0, not used

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*3000;  // delay between updates, in milliseconds

void setup() {
  Serial.begin(9600); // Start the serial library:
    
  // Welcome message
  Serial.println("Starting Emoncms");
  
  Serial.println("Acquiring Network connection, Please wait ...");
  Ethernet.begin(mac); // Start Ethernet
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.println("Please check connection and Restart Arduino");
    for (;;);
  }
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP()); // Displays Local IP to Serial
}

// the loop function runs over and over again forever
void loop() {
   // Read Sensors
  ampValue = analogRead(ampPin);
  ampValue = ((ampValue - 513) * 75.76 / 1023);
  voltValue = analogRead(voltPin) / 43.215;
  // Amps X Volts
  wattValue = ampValue * voltValue;
// -----------------------------------------------    
// if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("Disconnecting...");
    client.stop();
  }
  // if you're not connected, and at least <postingInterval> milliseconds have
  // passed sinceyour last connection, then connect again and
  // send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
// ------------------------------------------------
    //Print values (debug)
    debugPrint();
    //send values
    sendData();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}
// This method is for debugging only. Can be commented out.
void debugPrint(){
  // Print values (Debug)
  Serial.println("Solar Panel Readings");
  Serial.print("Amps: ");
  Serial.print(ampValue);
  Serial.print(" ; Volts: ");
  Serial.print(voltValue);
  Serial.print(" ; Power: ");
  Serial.println(wattValue);
  //Serial.print("Battery Voltage : ");
  //Serial.println(battValue);
}
// this method makes a HTTP connection to the server:
void sendData() {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    // send the HTTP GET request:
    client.print("GET /api/post?apikey=");
    client.print(apikey);
    if (node > 0) {
      client.print("&node=");
      client.print(node);
    }
    client.print("&json={volts");
    client.print(":");
    client.print(voltValue);    
    client.print(",amps:");
    client.print(ampValue);
    client.print(",power:");
    client.print(wattValue);   
    client.println("} HTTP/1.1");
    client.println("Host:emoncms.org");
    client.println("User-Agent: Arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("Connection failed");
    Serial.println("Disconnecting...");
    client.stop();
  }
  }
