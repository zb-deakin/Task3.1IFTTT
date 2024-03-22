#include <ArduinoHttpClient.h>  // http client  (arduino offical library)
#include <WiFiNINA.h>           // wifi library (arduino offical library)
#include "secrets.h"            // secrets for wifi and IFTT

// setup wifi
const char wifiAccessPointName[] = WIFI_NAME;
const char wifiPassword[] = WIFI_PASSWORD;
const int wifiReconnectDelay = 5000;
int wifiConnectionStatus = WL_IDLE_STATUS;
String connection = "Wifi connecting to '" + String(wifiAccessPointName) + "'";
// wifi client
WiFiClient TCP_wifiClient;

// setup http client for IFTT
const char serverAddress[] = "maker.ifttt.com";
String credentials = "/trigger/" + String(IFTTT_EVENT) + "/with/key/" + String(IFTTT_KEY);
String queryStringValue;
// http client
HttpClient httpClientTcp = HttpClient(TCP_wifiClient, serverAddress);

// setup light sensor variables
const int lightSensorPin = A0;
const int notBrightEnough = 500;
bool wasReceivingSunlightOnLastSensorReading = false;
int lightDetected;

// space out readings to avoid spamming with notifications
const int delayBetweenReadings = 5000;

void setup() {
  // wait until serial is ready
  Serial.begin(9600);
  while (!Serial)
    ;

  // wait until wifi connection is ready
  while (wifiConnectionStatus != WL_CONNECTED) {
    Serial.println(connection);

    wifiConnectionStatus = WiFi.begin(wifiAccessPointName, wifiPassword);
    // wait 5 seconds for connection to start
    delay(wifiReconnectDelay);
  }

  Serial.println("*** CONNECTED TO WIFI ***");
}

void loop() {
  // get light-level reading
  lightDetected = analogRead(lightSensorPin);
  // give it a moment to convert from analogue to digital value
  delay(10);
  // print captured value
  Serial.println("\n>>> Light sensor value: " + String(lightDetected));

  // collect status
  const bool isReceivingSunlightNow = lightDetected > notBrightEnough;
  const bool stillSunny_noChangeSinceLastRead = wasReceivingSunlightOnLastSensorReading && isReceivingSunlightNow;
  const bool stillDark_noChangeSinceLastRead = !wasReceivingSunlightOnLastSensorReading && !isReceivingSunlightNow;

  // no change detected - wait for the next sensor read
  if (stillSunny_noChangeSinceLastRead || stillDark_noChangeSinceLastRead) {
    Serial.println("No change - still " + String(isReceivingSunlightNow ? "sunny" : "dark"));
    delay(delayBetweenReadings);

    // return early
    return;
  }

  wasReceivingSunlightOnLastSensorReading = isReceivingSunlightNow;

  // prepare data for IFTT
  queryStringValue = String(isReceivingSunlightNow ? "HGH_BRIGHTNESS" : "LOW_BRIGHTNESS") + String("_" + String(lightDetected));
  Serial.println("CHANGE DETECTED: " + queryStringValue);

  // send a http GET request to the IFTTT webhook
  httpClientTcp.get(credentials + "?value1=" + queryStringValue);

  // process the response
  const int statusCode = httpClientTcp.responseStatusCode();
  String response = httpClientTcp.responseBody();

  // print the response
  Serial.println("\nHTTP response code: " + String(statusCode));
  Serial.println("IFTTT response: " + response);

  // wait before getting another reading to prevent spamming of notifications
  delay(delayBetweenReadings);
}
