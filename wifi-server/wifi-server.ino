#include <WiFi.h>
<<<<<<< Updated upstream
=======
#include <vector>
#define PIN_R 27
#define PIN_G 26
#define PIN_B 25
#define CH_R 0
#define CH_G 1
#define CH_B 2
>>>>>>> Stashed changes

const uint8_t frequency = 5000;
const uint8_t resolution = 8;
const char* ssid = "csy";
const char* password = "55555555";
static volatile int red_val = 255;
static volatile int green_val = 255;
static volatile int blue_val = 255;
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  connectWiFi(ssid, password);
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server is started at port 80");
  hardwareSetup();
}

void loop() {
  WiFiClient client = server.accept();
  if (client) {
    String req = getReq(client);
    handleReq(req);
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

void handleReq(String req) {
  if (req == "") {
    Serial.println("Empty request, do nothing");
  }
  // do something
<<<<<<< Updated upstream
  if (req.indexOf("/on") >= 0) {
    
=======
  if (command == "/on") {

  } else if (command == "/off") {

  } else if (command == "") {

  }
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
>>>>>>> Stashed changes
  }
}

<<<<<<< Updated upstream
=======
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


>>>>>>> Stashed changes
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

// https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/ledc.html
void hardwareSetup() {
  // Important to delay to prevent race condition!
  Serial.print(ledcAttachChannel(PIN_R, frequency, resolution, CH_R));
  delay(10);  
  Serial.print(ledcAttachChannel(PIN_G, frequency, resolution, CH_G));
  delay(10);
  Serial.print(ledcAttachChannel(PIN_B, frequency, resolution, CH_B));
  delay(10);

  // ledcAttach(PIN_R, frequency, resolution);
  // ledcAttach(PIN_G, frequency, resolution);
  // ledcAttach(PIN_B, frequency, resolution);

  // ledcAttachPin(PIN_R, CH_R);
  // ledcAttachPin(PIN_G, CH_G);
  // ledcAttachPin(PIN_B, CH_B);

  ledcWrite(PIN_R, red_val);
  ledcWrite(PIN_G, green_val);
  ledcWrite(PIN_B, blue_val);
}