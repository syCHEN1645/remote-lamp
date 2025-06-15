#include <WiFi.h>
#include <vector>
#define PIN_R 27
#define PIN_G 26
#define PIN_B 25
#define CH_R 0
#define CH_G 1
#define CH_B 2

const uint8_t frequency = 5000;
const uint8_t resolution = 8;
const char* ssid = "csy";
const char* password = "55555555";
volatile int red_val = 255;
volatile int green_val = 255;
volatile int blue_val = 255;
unsigned long time_sec = 0;
bool isTimed = false;
hw_timer_t *timer = NULL;
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
  // Serial.println(0);
  if (isTimed) {
    if (timerReadSeconds(timer) >= time_sec) {
      timerEnd(timer);
      Serial.printf("Time %l is up.", time_sec);
      isTimed = false;
      time_sec = 0;
    }
  }

  ledcWrite(PIN_R, red_val);
  ledcWrite(PIN_G, green_val);
  ledcWrite(PIN_B, blue_val);
  
  // Only true if a request is sent.
  if (client) {
    String req = getReq(client);
    Serial.println(1);
    String command = getCommand(req);
    Serial.println(2);
    handleReq(command);
    Serial.println(3);
    sendResponse(client, command);
    Serial.println(4);
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

void handleReq(String command) {
  if (command == "/on") {
    onLamp();
  } else if (command == "/off") {
    offLamp();
  } else if (command == "/reset") {
    resetLamp();
  } else if (command == "/timed") {
    timedLamp();
  } else {
    // default behaviour
  }
}

String getCommand(String req) {
  String command = "";
  // do something
  if (req.indexOf("/on") >= 0) {
    command = "/on";
  } else if (req.indexOf("/off") >= 0) {
    command = "/off";
  } else if (req.indexOf("/reset") >= 0) {
    command = "/reset";
  } else if (req.indexOf("/timed") >= 0) {
    command = "/timed";
  } else {
    command = "";
  }
  return command;
}

void onLamp() {
  Serial.println("On lamp");
  red_val = 255;
  green_val = 255;
  blue_val = 255;
  time_sec = 0;
  isTimed = false;
}

void offLamp() {
  Serial.println("Off lamp");
  red_val = 0;
  green_val = 0;
  blue_val = 0;
  time_sec = 0;
  isTimed = false;
}

void resetLamp() {
  Serial.println("Reset lamp");
  red_val = 0;
  green_val = 0;
  blue_val = 0;
  time_sec = 0;
  isTimed = false;
}

void timedLamp() {
  Serial.println("Timed lamp mode");
  red_val = 255;
  green_val = 255;
  blue_val = 255;
  // 
  time_sec = 12;
  isTimed = true;
  timer = timerBegin(1000);
}

void setColourLamp() {
  Serial.println("Set lamp colour mode");
  red_val = 255;
  green_val = 255;
  blue_val = 255;
}

std::vector<String> generateHeader(String command) {
  std::vector<String> res = {};
  res.push_back("HTTP/1.1 200 OK");
  res.push_back("Content-Type: application/json");
  res.push_back("Access-Control-Allow-Origin: *");
  res.push_back("Connection: close");

  return res;
}

std::vector<String> generateContent(String command) {
  std::vector<String> res = {};
  if (command == "/on") {
    res.push_back("{\"message\": \"Lamp turned on\",");
  } else if (command == "/off") {
    res.push_back("{\"message\": \"Lamp turned off\",");
  } else if (command == "/reset") {
    res.push_back("{\"message\": \"Lamp reset\",");
  } else if (command == "/timed") {
    res.push_back("{\"message\": \"Lamp is timed\",");
  } else if (command == "/colour") {
    res.push_back("{\"message\": \"Lamp changes colour\",");
  } else if (command == "") {
    res.push_back("{\"message\": \"This is a server to ESP32\",");
  } else {
    res.push_back("{\"message\": \"An unknow request was received.\",");
  }

  // String info = "{\"red\": \"" + String(red_val) + "\"}"
  res.push_back("\"R\": \"" + String(red_val) + "\",");
  res.push_back("\"G\": \"" + String(green_val) + "\",");
  res.push_back("\"B\": \"" + String(blue_val) + "\",");
  res.push_back("\"timed\": \"" + String(isTimed) + "\",");
  res.push_back("\"time\": \"" + String(time_sec) + "\"}");

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

// https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/ledc.html
void hardwareSetup() {
  // Important to delay to prevent race condition!
  ledcAttachChannel(PIN_R, frequency, resolution, CH_R);
  delay(10);  
  ledcAttachChannel(PIN_G, frequency, resolution, CH_G);
  delay(10);
  ledcAttachChannel(PIN_B, frequency, resolution, CH_B);
  delay(10);

  ledcWrite(PIN_R, red_val);
  ledcWrite(PIN_G, green_val);
  ledcWrite(PIN_B, blue_val);
}