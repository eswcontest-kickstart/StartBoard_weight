#include "HX711.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>

int status = WL_IDLE_STATUS;
// #include "arduino_secrets.h" 
char ssid[] = "RTCAR";        // your network SSID (name)
char pass[] = "409504504";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[256]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

WiFiUDP Udp;

// HX711 circuit wiring
const int RR_DOUT_PIN = 2;
const int RR_SCK_PIN = 3;
const int FF_DOUT_PIN = 5;
const int FF_SCK_PIN = 6;

// Calibration Factors
const float RR_cf = 19730.0;
const float FF_cf = 19810.0;


HX711 scale1;
HX711 scale2;


void setup() {
  
  Serial.begin(9600);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }


  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }
  Serial.println("Connected to WiFi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);


  scale1.begin(RR_DOUT_PIN, RR_SCK_PIN);
  scale2.begin(FF_DOUT_PIN, FF_SCK_PIN);

  scale1.set_scale(RR_cf);
  scale2.set_scale(FF_cf);
}

void loop() {

  float sVal1 = scale1.get_units();
  float sVal2 = scale2.get_units();

  if (sVal1 < 0.0) {
    sVal1 = 0.0;
  }

  if (sVal2 < 0.0) {
    sVal2 = 0.0;
  }

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
    Serial.println(sVal1, sVal2);

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 32);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
  
      // send a reply, to the IP address and port that sent us the packet we received
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      char temp[64];
      memset(temp, 0x00, 64);

      sprintf(temp, "{\"scale1\": %f, \"scale2\": %f}", sVal1, sVal2 );    
      Udp.write(temp); // ReplyBuffer);
      Udp.endPacket();

      delay(5);
      Udp.stop();

      delay(5);
      Serial.println("\nStarting connection to server...");
      // if you get a connection, report back via serial:
      Udp.begin(localPort);
      
      Serial.print("Listening on port ");
      Serial.println(localPort);      

  }
  
  
  delay(100);
}



void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


