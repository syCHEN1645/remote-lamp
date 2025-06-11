#include <WiFi.h>
#include <vector>

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
  if (client) {
    String req = getReq(client);
    handleReq(client, req);
  }
}

String getReq(WiFiClient client) {
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
  return req;
}

void handleReq(WiFiClient client, String req) {
  String command = "";
  if (req.indexOf("/on") >= 0) {
    command = "/on";
  } else if (req.indexOf("/off") >= 0) {
    command = "/off";
  } else {

  }
  executeCommand(command);
  sendResponse(client, command);
}

void executeCommand(String command) {
  // do something
}


std::vector<String> generateHeader(String command) {
  std::vector<String> res = {};
  res.push_back("HTTP/1.1 200 OK");
  if (command == "/on" || "/off") {
    res.push_back("Content-Type: application/json");
  } else if (command == "") {
    res.push_back("Content-Type: text/plain");
  }  
  res.push_back("Access-Control-Allow-Origin: *");
  res.push_back("Connection: close");

  return res;
}

std::vector<String> generateContent(String command) {
  std::vector<String> res = {};
  if (command == "/on") {
    res.push_back("{\"message\": \"Lamp turned on\"}");
  } else if (command == "/off") {
    res.push_back("{\"message\": \"Lamp turned off\"}");
  } else if (command == "") {
    res.push_back("This is a server to ESP32");
  } else {
  }

  return res;
}

void sendResponse(WiFiClient client, String command) {
  Serial.println("Sending response ...");
  std::vector<String> header = generateHeader(command);
  std::vector<String> content = generateContent(command);
  for (String str : header) {
    client.println(str);
  }
  client.println();
  for (String str : content) {
    client.println(str);
  }
  delay(10);
  client.stop();
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