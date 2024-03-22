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

  // connect to wifi
  Serial.println(connection);
  wifiConnectionStatus = WiFi.begin(wifiAccessPointName, wifiPassword);

  // wait until wifi is ready
  while (wifiConnectionStatus != WL_CONNECTED) {
    Serial.println(connection);

    wifiConnectionStatus = WiFi.begin(wifiAccessPointName, wifiPassword);

    // wait 5 seconds for connection to start
    delay(5000);
  }

  Serial.println("*** CONNECTED TO WIFI ***");
}

void loop() {
  // get light-level reading
  lightSensorValue = analogRead(lightSensorPin);
  // give it a moment to convert from analogue to digital value
  delay(10);
  // print captured value
  Serial.println(lightSensorValue);

  // trigger a GET request to IFTTT webook
  queryStringValue = String(lightSensorValue > brightnessBoundary ? "HGH_BRIGHTNESS" : "LOW_BRIGHTNESS") + String("_" + String(lightSensorValue));
  
  // send the http request
  httpClient.get(credentials + "?value1=" + queryStringValue);
  
  // process the response
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  // print the response
  Serial.println("HTTP response code: " + statusCode);
  Serial.println("IFTTT response: " + response);
  
  // space out the notifications to stop spamming of the device
  Serial.println("Pause for 20 seconds");
  delay(20000);
}
