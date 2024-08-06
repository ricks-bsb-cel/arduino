/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
 *************************************************************
  Blynk.Edgent implements:
  - Blynk.Inject - Dynamic WiFi credentials provisioning
  - Blynk.Air    - Over The Air firmware updates
  - Device state indication using a physical LED
  - Credentials reset using a physical Button
 *************************************************************/

/* Fill in information from your Blynk Template here */
/* Read more: https://bit.ly/BlynkInject */
#define BLYNK_TEMPLATE_ID "TMPL2X9ejTS6V"
#define BLYNK_TEMPLATE_NAME "Demonstração Principal"
// #define BLYNK_AUTH_TOKEN "COojb25_fI1_SueTDL0R_SHdC8LAk86K"

#define BLYNK_FIRMWARE_VERSION "0.1.6"
#define BLYNK_PRINT Serial

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
#define USE_NODE_MCU_BOARD

#include "BlynkEdgent.h"

#include <DHT.h>

uint32_t  staticIP;
uint32_t  staticMask;
uint32_t  staticGW;
uint32_t  staticDNS;
uint32_t  staticDNS2;

BlynkTimer timer;

// DHT11 Sensor
DHT dht_sensor(D2, DHT11);

void ReadLocalTempUmidity() {
  float temp = dht_sensor.readTemperature();
  Serial.println("Reading Humidity...");
  float humi = dht_sensor.readHumidity();

  if (!isnan(temp)) {
    Serial.println(temp);
  }

  if (!isnan(humi)) {
    Serial.println(humi);
  }
}

void BlinkTimer() {
  Serial.println("BlinkTimer v0.1.6");
  ReadLocalTempUmidity();
}

void setup()
{
  Serial.begin(74880);
  delay(100);

  BlynkEdgent.begin();
  timer.setInterval(5000L, BlinkTimer);  // 1 Minuto
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}

