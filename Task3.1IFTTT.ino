// #include <NTPClient.h>          // network time client (arduino offical library)
#include <NTP.h>                // NTP library by Stefan Staub
#include <ArduinoHttpClient.h>  // http client         (arduino offical library)
#include <WiFiNINA.h>           // wifi library        (arduino offical library)
#include "secrets.h"            // secrets for wifi and IFTT

// setup wifi
char wifiAccessPointName[] = WIFI_NAME;
char wifiPassword[] = WIFI_PASSWORD;
String connection = "Wifi connecting to '" + String(wifiAccessPointName) + "'";
int wifiConnectionStatus = WL_IDLE_STATUS;
// wifi clients
WiFiClient TCP_wifiClient;  // TCP for IFTT
WiFiUDP UDP_wifiClient;     // UDP for ntp

// setup http client for IFTT
char serverAddress[] = "maker.ifttt.com";
String credentials = "/trigger/" + String(IFTTT_EVENT) + "/with/key/" + String(IFTTT_KEY);
String queryStringValue;
// http client
HttpClient httpClientTcp = HttpClient(TCP_wifiClient, serverAddress);

// network time client
// NTPClient timeClientUdp(UDP_wifiClient);
NTP ntp(UDP_wifiClient);
int dayOfMonth;

// setup light sensor variables
int lightSensorPin = A0;
int lightDetected;
int notBrightEnough = 500;
unsigned long timeSinceArduinoWasStarted;

bool wasReceivingSunlightOnLastSensorReading = false;
bool isReceivingSunlightNow = false;

void setup() {
  // wait until serial is ready
  Serial.begin(9600);
  while (!Serial)
    ;

  // wait until wifi connection is ready
  Serial.println(connection);
  WiFi.begin(wifiAccessPointName, wifiPassword);
  wifiConnectionStatus = WiFi.begin(wifiAccessPointName, wifiPassword);
  while (wifiConnectionStatus != WL_CONNECTED) {
    Serial.println(connection);

    wifiConnectionStatus = WiFi.begin(wifiAccessPointName, wifiPassword);

    // wait 5 seconds for connection to start
    delay(5000);
  }

  Serial.println("*** CONNECTED TO WIFI ***");

  // timeClientUdp.begin();
  ntp.timeZone(8);
  ntp.isDST(false);
  ntp.begin();
  Serial.println("*** NTP client ready ***");
  timeSinceArduinoWasStarted = millis();
}

void loop() {
  ntp.update();
  // timeClientUdp.update();
  // Serial.println(timeClientUdp.getFormattedTime());
  // Serial.println(timeClientUdp.);
  if (ntp.day() != dayOfMonth) {
    dayOfMonth = ntp.day();
  };


  // detect when light level first hits for the day
  // then detect when light level tapers off more than 10 minutes

  Serial.println(ntp.day());
  // Serial.println(ntp.ntp());
  Serial.println(ntp.formattedTime("%A %T"));
  Serial.println(ntp.formattedTime("%d. %B %Y"));


  delay(1000);

  return;




  // return;

  // get light-level reading
  lightDetected = analogRead(lightSensorPin);
  // give it a moment to convert from analogue to digital value
  delay(10);
  // print captured value
  Serial.println("Light sensor value: " + String(lightDetected));

  isReceivingSunlightNow = lightDetected > notBrightEnough;

  // no change detected - wait for the next sensor read
  if (
    // was sunny, still sunny
    (wasReceivingSunlightOnLastSensorReading && isReceivingSunlightNow) ||
    // was dark, still dark
    (!wasReceivingSunlightOnLastSensorReading && !isReceivingSunlightNow)) {
    Serial.println("No change - still " + isReceivingSunlightNow ? "sunny" : "dark");
    delay(60000);
    return;
  }

  // prepare data for IFTT
  queryStringValue = String(isReceivingSunlightNow ? "HGH_BRIGHTNESS" : "LOW_BRIGHTNESS") + String("_" + String(lightDetected));

  // send a http GET request to the IFTTT webhook
  httpClientTcp.get(credentials + "?value1=" + queryStringValue);

  // process the response
  int statusCode = httpClientTcp.responseStatusCode();
  String response = httpClientTcp.responseBody();

  // print the response
  Serial.println("HTTP response code: " + String(statusCode));
  Serial.println("IFTTT response: " + response);

  // space out the notifications to stop spamming of the device
  Serial.println("Pause for 20 seconds");
  delay(20000);
}
