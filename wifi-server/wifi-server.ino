#include <WiFi.h>

#include <WebServer.h>
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
volatile unsigned long time_sec = 0;
volatile bool isTimed = false;
hw_timer_t *timer = NULL;
WiFiServer server(80);
IPAddress ip = WiFi.localIP();

const String html_code PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>
        LAMP CONTROLLER
    </title>
    <style>
        body {
            font-family: 'Courier New', Courier, monospace;
            text-align: center;
            padding: 10vw;
        }
        button, input[type=range] {
            padding: 10px 30px;
            font-size: 16px;
            margin: 10px;
        }
        .slider-container {
            margin-bottom: 1em;
        }
    </style>
</head>

<body>
    <h1>
        Main Page
    </h1>
    <button id="btnOn">
        On
    </button>
    <button id="btnOff">
        Off
    </button>
    <button id="btnReset">
        Reset
    </button>

    <div class="slider-container">
        <label for="sliderR">Red: <span id="valR">0</span></label><br>
        <input type="range" id="sliderR" min="0" max="255" value="0"><br>
    </div>

    <div class="slider-container">
        <label for="sliderG">Green: <span id="valG">0</span></label><br>
        <input type="range" id="sliderG" min="0" max="255" value="0"><br>
    </div>

    <div class="slider-container">
        <label for="sliderB">Blue: <span id="valB">0</span></label><br>
        <input type="range" id="sliderB" min="0" max="255" value="0"><br>
    </div>

    <div class="slider-container">
        <label for="entryTime">Turn off after (minutes): </label><br>
        <input type="number" id="entryTime" min="1" max="999" placeholder="0"><br>
        <button id="btnSetTime">Set Timer</button>
    </div>

    <script type="module">
        "use strict"
        const CMD = {
            ON: 'on',
            OFF: 'off',
            RGB: 'colour',
            TIME: 'timed',
            RST: 'reset',
        };
        const ipAddress = ")rawliteral" + ip.toString() + R"rawliteral(";
        
        function sendCommand(command) {
            // load is different by different commands
            let load;
            if (command == CMD.RGB) {
                load = [
                    parseInt(document.getElementById(`sliderR`).value),
                    parseInt(document.getElementById(`sliderG`).value),
                    parseInt(document.getElementById(`sliderB`).value)
                ];
            } else if (command == CMD.TIME) {
                // send time
                load = [parseInt(document.getElementById(`entryTime`).value)];
            } else {
                load = [];
            }

            fetch(`${ipAddress}/${command}/${load}`).then(
                response => {
                    if (!response.ok) {
                        throw new Error("Response is not ok.");
                    }
                    // return response.text();
                    return response.json();
                }
            ).then(
                data => {
                    console.log("Message: ESP returns a message: ", data);
                    // update information on UI display to match the actual status of device
                    setSlider("R", parseInt(data.R));
                    setSlider("G", parseInt(data.G));
                    setSlider("B", parseInt(data.B));
                }
            ).catch(
                error => {
                    console.error("Error: ", error);
                }
            );
        }

        // given a number val, set the slider value and text to match val
        function setSlider(colour, val) {
            document.getElementById(`slider${colour}`).value = val;
            document.getElementById(`val${colour}`).textContent = val;
        }

        // (when the slider is moved) send updated colour to device
        function updateSlider(colour) {
            // update text number to match slider position/value
            const val = document.getElementById(`slider${colour}`).value;
            document.getElementById(`val${colour}`).textContent = val;
            // send RGB value to device
            sendCommand(CMD.RGB);
        }

        window.addEventListener("load", () => sendCommand(CMD.ON));

        document.getElementById("btnOn").addEventListener("click", () => sendCommand(CMD.ON));
        document.getElementById("btnOff").addEventListener("click", () => sendCommand(CMD.OFF));
        document.getElementById("btnReset").addEventListener("click", () => sendCommand(CMD.RST));
        document.getElementById("btnSetTime").addEventListener("click", () => sendCommand(CMD.TIME));

        document.getElementById("sliderR").addEventListener("input", () => updateSlider('R'));
        document.getElementById("sliderG").addEventListener("input", () => updateSlider('G'));
        document.getElementById("sliderB").addEventListener("input", () => updateSlider('B'));
    </script>
</body>
</html>
)rawliteral";

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
  } else if (req.indexOf("/colour") >= 0) {
    command = "/colour";
  } else if (req.indexOf("GET / ") >= 0 || req.indexOf("GET /HTTP") >= 0) {
    command = "/";
  } else {
    command = "/other";
  }
  return command;
}

std::vector<int> getCommandData(String req) {
  // example: GET /0.0.0.0/colour/255,255,160 HTTP/1.1
  String substr = req.substring(req.indexOf(" "), req.lastIndexOf(" "));
  substr = substr.substring(substr.lastIndexOf("/") + 1, substr.length());
  std::vector<int> data = {};
  while (!substr.isEmpty()) {
    int end = substr.indexOf(",");
    if (end == -1) {
      end = substr.length();
    }
    int number = substr.substring(0, end).toInt();
    substr = substr.substring(end + 1, substr.length());
    data.push_back(number);
    Serial.println(number);
  }
  return data;
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

void timedLamp(String req) {
  Serial.println("Timed lamp mode");
  // time_sec = 12;
  std::vector<int> data = getCommandData(req);
  time_sec = data.back();
  data.pop_back();
  isTimed = true;
  timer = timerBegin(1000);
}

void setColourLamp(String req) {
  Serial.println("Set lamp colour mode");
  std::vector<int> data = getCommandData(req);
  blue_val = data.back();
  data.pop_back();
  green_val = data.back();
  data.pop_back();
  red_val = data.back();
  data.pop_back();
}

std::vector<String> generateHeader(String command) {
  std::vector<String> res = {};
  res.push_back("HTTP/1.1 200 OK");
  if (command == "/") {
    res.push_back("Content-Type: text/html");
  } else {
    res.push_back("Content-Type: application/json");
  }
  res.push_back("Access-Control-Allow-Origin: *");
  res.push_back("Connection: close");

  return res;
}

std::vector<String> generateContent(String command) {
  std::vector<String> res = {};
  if (command == "/") {
    res.push_back(html_code);
    return res;
  }
  
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
  } else if (command == "/other") {
    res.push_back("{\"message\": \"This is a server to ESP32\",");
  } else {
    res.push_back("{\"message\": \"An unknow request was received.\",");
  }

  res.push_back("\"R\": \"" + String(red_val) + "\",");
  res.push_back("\"G\": \"" + String(green_val) + "\",");
  res.push_back("\"B\": \"" + String(blue_val) + "\",");
  res.push_back("\"timed\": \"" + String(isTimed) + "\",");
  res.push_back("\"time\": \"" + String(time_sec) + "\"}");

  return res;
}

void sendResponse(WiFiClient client, String req) {
  String command = getCommand(req);
  Serial.println("Sending " + command + " response ...");

  std::vector<String> header = generateHeader(command);
  std::vector<String> content = generateContent(command);
  for (String str : header) {
    client.println(str);
    Serial.println(str);
  }
  client.println();
  for (String str : content) {
    client.println(str);
    Serial.println(str);
  }
  delay(10);
  client.stop();
}

void handleReq(String req) {
  String command = getCommand(req);
  if (command == "/on") {
    onLamp();
  } else if (command == "/off") {
    offLamp();
  } else if (command == "/reset") {
    resetLamp();
  } else if (command == "/timed") {
    timedLamp(req);
  } else if (command == "/colour") {
    setColourLamp(req);
  } else {
    // default behaviour
    Serial.println("No action to be done.");
  }
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
      Serial.println(timerReadSeconds(timer));
      Serial.println(time_sec);
      timerEnd(timer);
      Serial.printf("Time %l is up.", time_sec);
      offLamp();
    }
  }

  ledcWrite(PIN_R, red_val);
  ledcWrite(PIN_G, green_val);
  ledcWrite(PIN_B, blue_val);
  
  // Only true if a request is sent.
  if (client) {
    String req = getReq(client);
    handleReq(req);
    sendResponse(client, req);
  }
}