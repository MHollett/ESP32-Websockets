#include <Arduino.h>
#include <WiFiMulti.h>

#define WIFI_SSID "BELL124"
#define WIFI_PASSWORD "A72E1E66A5F2"

WiFiMulti wifiMulti; // Initiate wifi multi instance

void setup() {
  Serial.begin(921600);
  pinMode(LED_BUILTIN, OUTPUT); // Set pin mode of the built in LED

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) { // Continues until the controller connects to wifi
    delay(100);
  }

  Serial.println("Connected");
}

void loop() {
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED); // Light indicates if the wifi is connected
}
