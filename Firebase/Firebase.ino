/*
  Make sure your Firebase project's '.read' and '.write' rules are set to 'true'. 
  Ignoring this will prevent the MCU from communicating with the database. 
  For more details- https://github.com/Rupakpoddar/ESP8266Firebase 

  Download the Android app from- https://play.google.com/store/apps/details?id=com.rupak.firebaseremote
  Online remote (Works with any web browser)- https://rupakpoddar.github.io/Firebase-automation-web-interface/
  Use Python to control devices- https://github.com/Rupakpoddar/Firebase-with-python
*/

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>

#define _SSID "brisa-2154254"                                                  // Your WiFi SSID
#define _PASSWORD "syvxfcxh"                                                   // Your WiFi Password
#define REFERENCE_URL "https://esp8266testricks-default-rtdb.firebaseio.com/"  // Your Firebase project reference url

Firebase firebase(REFERENCE_URL);

void setup() {
  Serial.begin(74880);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(_SSID);
  WiFi.begin(_SSID, _PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  firebase.json(true);  // Make sure to add this line.
}

void loop() {
  String data = firebase.getString("teste");  // Get data from database.

  // Deserialize the data.
  // Consider using the Arduino Json assistant- https://arduinojson.org/v6/assistant/
  const size_t capacity = JSON_OBJECT_SIZE(5) + 60;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, data);

  String Device1 = doc["Device1"];
  String Device2 = doc["Device2"];

  // Print data
  Serial.println("Device 1: " + Device1);
  Serial.println("Device 2: " + Device2);
  Serial.println("");
}