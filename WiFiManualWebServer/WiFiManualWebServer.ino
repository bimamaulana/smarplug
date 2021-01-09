/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "Syamsul family"
#define STAPSK  "Syamsul@1963"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // prepare LED
  pinMode(D5, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D6, OUTPUT);
  digitalWrite(D4, LOW);
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println(F("new client"));

  client.setTimeout(5000); // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(req);

  // Match the request
  int val1;
  int val2;
  int val3;
  if (req.indexOf(F("/gpio/reset")) != -1) {
    val1 = LOW;
    val2 = LOW;
    val3 = LOW;
  } else if (req.indexOf(F("/gpio/d5")) != -1) {
    val1 = HIGH;
    val2 = digitalRead(D4);
    val3 = digitalRead(D6);
  } else if (req.indexOf(F("/gpio/d4")) != -1) {
    val1 = digitalRead(D5);
    val2 = HIGH;
    val3 = digitalRead(D6);
  } else if (req.indexOf(F("/gpio/d6")) != -1) {
    val3 = HIGH;
    val2 = digitalRead(D4);
    val1 = digitalRead(D5); 
  } else {
    Serial.println(F("invalid request"));
    val1 = digitalRead(D5);
    val2 = digitalRead(D4);
    val3 = digitalRead(D6);
  }

  // Set LED according to the request
  digitalWrite(D5, val1);
  digitalWrite(D4, val2);
  digitalWrite(D6, val3);
  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n"));
  client.print(F("D5 is now "));
  client.print((val1) ? F("high") : F("low"));
  client.print(F("\r\n"));
  client.print(F("D4 is now "));
  client.print((val2) ? F("high") : F("low"));
  client.print(F("\r\n"));
  client.print(F("D6 is now "));
  client.print((val3) ? F("high") : F("low"));
  client.print(F("<br><br><button onClick=\"window.location.href='http://"));
  client.print(WiFi.localIP());
  client.print("/gpio/d5';\">D5 ON</button>");
  client.print(F("<br><br><button onClick=\"window.location.href='http://"));
  client.print(WiFi.localIP());
  client.print("/gpio/d4';\">D4 ON</button>");
  client.print(F("<br><br><button onClick=\"window.location.href='http://"));
  client.print(WiFi.localIP());
  client.print("/gpio/d6';\">D6 ON</button>");
  client.print(F("\r\n"));
  client.print(F("<br><br><button onClick=\"window.location.href='http://"));
  client.print(WiFi.localIP());
  client.print("/gpio/reset';\">reset</button>");
  client.print(F("\r\n"));
  
 
  client.print(F("</html>"));

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  Serial.println(F("Disconnecting from client"));
}
