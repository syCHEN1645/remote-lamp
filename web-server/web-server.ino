#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "csy";
const char* password = "55555555";
WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  connectWiFi(ssid, password);
  startServer();
  
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server is started at port 80");
}

void loop() {
  server.handleClient();
}


void startServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/on", HTTP_GET, handleOn);
  server.on("/off", HTTP_GET, handleOff);
}

void handleRoot() {
  // status code, reponse type, response body
  server.send(200, "text/plain", "Root");
}

void handleOn() {
  // status code, reponse type, response body
  server.send(200, "application/json", "{\"status\":\"on\"}");
}

void handleOff() {
  // status code, reponse type, response body
  server.send(200, "application/json", "{\"status\":\"off\"}");
}

void connectWiFi(const char* wifiName, const char* wifiPassword) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnection success");
}