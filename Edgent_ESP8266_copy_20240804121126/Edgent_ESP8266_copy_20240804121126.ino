
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
#define BLYNK_TEMPLATE_ID "TMPL2iGv0-zOt"
#define BLYNK_TEMPLATE_NAME "Edgent Test Template"

#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
#define USE_NODE_MCU_BOARD

#include <Arduino.h>
#include <Blynk.h>
#include <BlynkSimpleEsp8266.h>

#include "BlynkEdgent.h"


class BuildInLed {
  bool isOn = false;
public:
  BuildInLed() {
    pinMode(LED_BUILTIN, OUTPUT);
  }
  void On() {
    digitalWrite(LED_BUILTIN, LOW);
  }
  void Off() {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  void Alternate() {
    if (isOn) {
      Off();
    } else {
      On();
    }
    isOn = !isOn;
  }
};

BlynkTimer timer;
BuildInLed led;

void BlinkTimer() {
  Serial.println("BlinkTimer!");
  led.Alternate();
}

void setup()
{
  Serial.begin(74880);
  delay(100);

  BlynkEdgent.begin();

  // Here your Arduino connects to the Blynk Cloud.
  timer.setInterval(5000L, BlinkTimer);  // 1 Minuto
}

void loop() {
  BlynkEdgent.run();
  Blynk.run();
}

