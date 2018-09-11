#include "arduino_secrets.h"

#include <SPI.h>
#include <WiFi101.h>

WiFiClient client;                // instance of the WIFi client library
int status = WL_IDLE_STATUS;      // WiFi status
IPAddress server(192,168,1,3);

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

const int sendInterval = 100;     // minimum time between messages to the server
const int debounceInterval = 15;  // used to smooth out pushbutton readings
int prevButtonState = 0;          // previous state of the pushbutton
long lastTimeSent = 0;            // timestamp of the last server message

#define potPinX A0
#define potPinY A1

const int connectButton = 0;      // the pushbutton for connecting/disconnecting
const int connectionLED = 5;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
//   initialize digital inputs and outputs:
//  pinMode(connectionLED, OUTPUT);
  pinMode(connectButton, INPUT_PULLUP);
  while (!Serial); // wait for serial port to connect.

  
  // Print WiFi MAC address:
  printMacAddress();

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 3 seconds for connection:
    delay(3000);
  }

  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop(){
    // message to send to the server:
  String message = "";                 
  // note the current time in milliseconds:
  long now = millis();
  // check to see if the pushbutton's pressed:
  boolean buttonPushed = buttonRead(connectButton);

  // if the button's just pressed:
  if (buttonPushed) {
    // if the client's connected, disconnect:
    if (client.connected()) {
      Serial.println("disconnecting");
      client.print("x");
      client.stop();
    } // if the client's disconnected, try to connect:
    else {
      Serial.println("connecting");
      client.connect(server, 8080);
    }
  }
    
  int xVal = map(analogRead(potPinX), 0, 780, 0, 100);
  int yVal = map(analogRead(potPinY), 0, 780, 0, 100);

//  Serial.print("x-value: ");
//  Serial.print(xVal);
//  Serial.print(" | y-value: ");
//  Serial.println(yVal);

  int repX = intensity(xVal);
  int repY = intensity(yVal);

  String comX = "";
  String comY = "";

  for (int i = 0; i < abs(repX); i++){
    if (repX < 0){
      comX += "l";
    }
    if (repX > 0){
      comX += "r";
    }
  }
  

    for (int i = 0; i < abs(repY); i++){
    if (repY < 0){
      comY += "d";
    }
    if (repY > 0){
      comY += "u";
    }
  }
  
  message += comX;
  message += comY;

  if (client.connected()                     // if the client's connected
      && now - lastTimeSent > sendInterval
      && message != 0){   // and the send interval has elapased              
    client.print(message);                   // send a message
    Serial.print(message);
    //save this moment as last time you sent a message:
    lastTimeSent = now;
  }

  // set the connection LED based on the connection state:
  digitalWrite(connectionLED, client.connected());

    // if there's incoming data from the client, print it serially:
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
}

int intensity(int slideVal){
  int repeater = 0;
  if (slideVal < 100){
    repeater = 3;
  }
  if (slideVal < 97){
    repeater = 2;
  }
  if (slideVal < 92){
    repeater = 1;
  }
  if (slideVal < 85){
    repeater = 0;
  }
  if (slideVal < 50){
    repeater = -1;
  }
  if (slideVal < 30){
    repeater = -2;
  }
  if (slideVal < 10){
    repeater = -3;
  }
  return repeater;
}

void printMacAddress() {
  // the MAC address of your WiFi shield
  byte mac[6];

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}

boolean buttonRead(int thisButton) {
  boolean result = false;
  int currentState = digitalRead(thisButton); // current button state

  if (currentState != prevButtonState         // if the button's changed
      && currentState == LOW) {               // and it's low
    result = true;                            // result is true
  }

  delay(debounceInterval);                    // debounce delay
  prevButtonState = currentState;             // save the button state for next time
  return result;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
