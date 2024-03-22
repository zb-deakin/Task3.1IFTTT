#include <ArduinoHttpClient.h>  // arduino http client
#include <WiFiNINA.h>           // arduino wifi library
#include "secrets.h"            // secrets for wifi and IFTT

// setup wifi
char wifiAccessPointName[] = WIFI_NAME;
char wifiPassword[] = WIFI_PASSWORD;
int wifiConnectionStatus = WL_IDLE_STATUS;
String connection = "Wifi connecting to '" + String(wifiAccessPointName) + "'";
// wifi client
WiFiClient wifiClient;

// setup http client for IFTT
char serverAddress[] = "maker.ifttt.com";
String credentials = "/trigger/" + String(IFTTT_EVENT) + "/with/key/" + String(IFTTT_KEY);
String queryStringValue;
// http client
HttpClient httpClient = HttpClient(wifiClient, serverAddress);

// setup light sensor variables
int lightSensorPin = A0;
int lightSensorValue;
int brightnessBoundary = 500;

void setup() {
  // wait until serial is ready
  Serial.begin(9600);
  while (!Serial)
    ;

  // wait until wifi connection is ready
  Serial.println(connection);
  wifiConnectionStatus = WiFi.begin(wifiAccessPointName, wifiPassword);
  while (wifiConnectionStatus != WL_CONNECTED)
    ;

  Serial.println("*** CONNECTED TO WIFI ***");
}

void loop() {
  // get light-level reading
  lightSensorValue = analogRead(lightSensorPin);
  // give it a moment to convert from analogue to digital value
  delay(10);
  // print captured value
  Serial.println("Light sensor value: " + String(lightSensorValue));

  // prepare data for IFTT
  queryStringValue = String(lightSensorValue > brightnessBoundary ? "HGH_BRIGHTNESS" : "LOW_BRIGHTNESS") + String("_" + String(lightSensorValue));

  // send a http GET request to the IFTTT webhook
  httpClient.get(credentials + "?value1=" + queryStringValue);

  // process the response
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  // print the response
  Serial.println("HTTP response code: " + String(statusCode));
  Serial.println("IFTTT response: " + response);

  // space out the notifications to stop spamming of the device
  Serial.println("Pause for 20 seconds");
  delay(20000);
}
