/* Template Definition */
#define BLYNK_TEMPLATE_ID "TMPL2dhm-xT37"
#define BLYNK_TEMPLATE_NAME "4 Relay Module"

#define BLYNK_FIRMWARE_VERSION "0.1.1"
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#include "LcdLog.h"
LcdLog Lcd("HeySensa " + String(BLYNK_FIRMWARE_VERSION), true);
#define ShowOnLcd(p1) { Lcd.Log(p1); }

#include "BlynkEdgent.h"

BlynkTimer timer;

void BlinkTimer(){
  DEBUG_PRINT("Timer!");
}

void setup()
{
  ShowOnLcd("Starting");
  Serial.begin(74880);
  delay(100);

  BlynkEdgent.begin();
  timer.setInterval(10000L, BlinkTimer);  // 10 Seconds
  ShowOnLcd("Setup ready");
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}

