/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL2X9ejTS6V"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "NOczROi0KHXkLDeP-j4RK4bCn98So93Z"

#include <Arduino.h>
#include <ArduinoJson.h>  // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <ArduinoJson.hpp>

#include <vector>
#include <iostream>
#include <sstream>
#include <string>

#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>  // Include for HTTPS

#include <eeboom.h>

#include <BlynkSimpleEsp8266.h>

#include "Util.h"
#include "BuildInLed.h"
#include "LcdFreeWrite.h"
#include "WiFiNetwork.h"
#include "Portal.h"
// #include "rtdbReles.h"


#define BLYNK_PRINT Serial /* Comment this out to disable prints and save space */


#define SERIAL_BAUD 74880  // Use este valor para ver também as msgs da própria placa

BlynkTimer timer;
BuildInLed buildLed;
WiFiNetwork Net(&timer);
LcdFreeWrite Lcd("HeySensa");  // O False desabilita o display, para o caso dele não existir. As msgs serão enviadas para a porta Serial
Portal PortalHtml(&Net, &Lcd, "HeySensa");
// rtdbReles Reles(&Net, &Lcd);

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();

  // Update state
  Blynk.virtualWrite(V1, value);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
}


void setup() {
  Lcd.Log("Starting...");
  Serial.begin(SERIAL_BAUD);  // Usado para comunicação com a USB e debug print
  buildLed.Off();
  PortalHtml.Setup("HeySensa");
}

void loop() {
  Blynk.run();
  timer.run();

  PortalHtml.Loop();
  // Reles.Loop();
}
