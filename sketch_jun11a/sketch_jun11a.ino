#include<WiFi.h>

const char* ssid = "csy";
const char* password = "55555555";
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  connectWiFi(ssid, password);
  
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server is started at port 80");
}

void loop() {
  
  WiFiClient client = server.accept();
  if (!client) {
    return;
  }
  // assertion: client is no null
  String req = "";
  while (client.connected()) {
    char c = client.read();
    req += c;
    // "\n" is the last char of a http request
    if (c == '\n') {
      break;
    }
  }
  Serial.println(req);
}

void startServer() {
  
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