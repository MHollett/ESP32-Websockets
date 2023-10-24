#include <Arduino.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "BELL124"
#define WIFI_PASSWORD "A72E1E66A5F2"

#define WS_HOST "zhgp6rewr1.execute-api.us-east-1.amazonaws.com"
#define WS_PORT 443 // Default port for wss protocol
#define WS_URL "/dev"

#define JSON_DOC_SIZE 2048
#define MSG_SIZE 256

WiFiMulti wifiMulti; // Initiate wifi multi instance
WebSocketsClient wsClient; // Initiate websocket client object




// Formats a c string into a json error message and sends it back through the websocket
void sendErrorMessage(const char *error) {
  char msg[MSG_SIZE];

  sprintf(msg, "{\"action\":\"msg\",\"type\":\"error\",\"body\":\"%s\"}", error);

  wsClient.sendTXT(msg);
}

// Formats and sends an acknowledgement message
void sendOkMessage() {
  wsClient.sendTXT("{\"action\":\"msg\",\"type\":\"status\",\"body\":\"ok\"}");
}

// Converts a string value (an incoming message from the websocket) into an accepted pinMode constant
uint8_t toMode(const char *val) {
  if (strcmp(val, "output") == 0) {
    return OUTPUT;
  }

  if (strcmp(val, "input_pullup") == 0)
  {
    return INPUT_PULLUP;
  }

  return INPUT;
}

// When the websocket sends a valid message, this function handles the functioning based on that message
void handleMessage(uint8_t * payload) {
  // Message is expected in JSON
  StaticJsonDocument<JSON_DOC_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, payload);

  // Handles an error in the JSON parsing
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    sendErrorMessage(error.c_str()); // Sends back error over the websocket
    return;
  }

  // make sure the message type is a c string
  if (!doc["type"].is<const char*>()) {
    sendErrorMessage("Invalid message type format");
    return;
  }

  // handle a command message
  if (strcmp(doc["type"], "cmd") == 0) {
    // handle an invalid command
    if (!doc["body"].is<JsonObject>()) {
      sendErrorMessage("Invalid command body");
      return;
    }

    // handle a pinMode command
    if (strcmp(doc["body"]["type"], "pinMode") == 0) {
      pinMode(doc["body"]["pin"], toMode(doc["body"]["mode"]));
      sendOkMessage();
      return;
    }

    // handle a digitalWrite command
    if (strcmp(doc["body"]["type"], "digitalWrite") == 0) {
      digitalWrite(doc["body"]["pin"], doc["body"]["value"]);
      sendOkMessage();
      return;
    }

    // handle a digitalRead command
    if (strcmp(doc["body"]["type"], "digitalRead") == 0) {
      auto value = digitalRead(doc["body"]["pin"]);

      char msg[MSG_SIZE];
      sprintf(msg, "{\"action\":\"msg\",\"type\":\"output\",\"body\":%d}", value);
      wsClient.sendTXT(msg);

      return;
    }

    sendErrorMessage("Unsupported command type");
    return;
  }

  sendErrorMessage("Unsupported message type");
  return;
}



// Handles an incoming web socket event
void onWSEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("WS Connected");
      break;
    case WStype_DISCONNECTED:
      Serial.println("WS Disonnected");
      break;
      case WStype_ERROR:
      Serial.println("WS Error");
      break;
    case WStype_TEXT:
      Serial.printf("WS Message: %s\n", payload);

      // It will either be a read or write command for now


      break;
  }
}

void setup() {
  Serial.begin(921600);
  pinMode(LED_BUILTIN, OUTPUT); // Set pin mode of the built in LED

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) { // Continues until the controller connects to wifi
    delay(100);
  }

  Serial.println("Connected");

  wsClient.beginSSL(WS_HOST, WS_PORT, WS_URL, "", "wss");
  wsClient.onEvent(onWSEvent);
}

void loop() {
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED); // Light indicates if the wifi is connected
  wsClient.loop();
}
