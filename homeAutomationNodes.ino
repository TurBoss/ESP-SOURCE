#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <Bounce2.h>


const char* ssid     = "WLAN_8F";
const char* password = "pladur13lol";

const char* host = "192.168.0.20";
const int httpPort = 80;

const int node = 1;


const int  buttonPin = 4;
const int relayPin = 5;

int buttonState = LOW;

char msg[64];
char msgHost[64];
char msgLen[64];
char msgAgent[64];

Bounce debouncer = Bounce();

ESP8266WebServer server(1337);

void handleRoot() {
  server.send(200, "text/plain", "OK");
}

void handleON() {
  digitalWrite(relayPin, HIGH);
  buttonState = HIGH;
  server.send(200, "text/plain", "OK");
}

void handleOFF() {
  digitalWrite(relayPin, LOW);
  buttonState = LOW;
  server.send(200, "text/plain", "OK");
}

void setup() {

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);

  debouncer.attach(buttonPin);
  debouncer.interval(50);

  Serial.begin(115200);

  delay(1000);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.println("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  //set static ip part
  WiFi.config(
    IPAddress(192, 168, 0, 21),
    IPAddress(192, 168, 0, 1),
    IPAddress(255, 255, 255, 0)
  );

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/ON", handleON);
  server.on("/OFF", handleOFF);

  server.begin();

}

void loop() {

  handleInput();

  server.handleClient();
}

void handleInput() {

  debouncer.update();

  if ( debouncer.rose() ) {
    buttonState = !buttonState;
    digitalWrite(relayPin, buttonState);
    sprintf(msg, "{\"out\": \"%d\", \"state\": \"%d\"}", node, buttonState);
    sendData(msg);
  }
}

void sendData(String data) {

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
  }
  else {

    // We now create a URI for the request

    
    Serial.println("Requesting...");

    sprintf(msgHost, "Host: %s", host);
    sprintf(msgLen, "Content-Length: %d", data.length());
    sprintf(msgAgent, "User-Agent: ESP8266-NODE-%d", node);
    
    // This will send the request to the server

    Serial.println("@@@@@@@@@@@@@@@@");
    Serial.println("POST /control/turn HTTP/1.1");
    Serial.println(msgHost);
    Serial.println(msgLen);
    Serial.println("Accept-Encoding: gzip, deflate");
    Serial.println("Accept: */*");
    Serial.println(msgAgent);
    Serial.println("Connection: close");
    Serial.println("Content-Type: application/json");
    Serial.println();
    Serial.println(data);
    Serial.println("@@@@@@@@@@@@@@@@");
    
    client.println("POST /control/turn HTTP/1.1");
    client.println(msgHost);
    client.println(msgLen);
    client.println("Accept-Encoding: gzip, deflate");
    client.println("Accept: */*");
    client.println(msgAgent);
    client.println("Connection: keep-alive");
    client.println("Content-Type: application/json");
    client.println();
    client.println(data);
    
    delay(10);

    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.println(line);
    }

    Serial.println();
    Serial.println("closing connection");

  }
}

