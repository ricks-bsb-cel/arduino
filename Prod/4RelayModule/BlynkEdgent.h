
extern "C" {
#include "user_interface.h"

  void app_loop();
  void restartMCU();
}

#include "Settings.h"
#include <BlynkSimpleEsp8266_SSL.h>

#if defined(BLYNK_USE_LITTLEFS)
#include <LittleFS.h>
#define BLYNK_FS LittleFS
#elif defined(BLYNK_USE_SPIFFS)
#if defined(ESP32)
#include <SPIFFS.h>
#elif defined(ESP8266)
#include <FS.h>
#endif
#define BLYNK_FS SPIFFS
#endif
#if defined(BLYNK_FS) && defined(ESP8266)
#define BLYNK_FILE_READ "r"
#define BLYNK_FILE_WRITE "w"
#endif

#ifndef BLYNK_NEW_LIBRARY
#error "Old version of Blynk library is in use. Please replace it with the new one."
#endif

#if !defined(BLYNK_TEMPLATE_NAME) && defined(BLYNK_DEVICE_NAME)
#define BLYNK_TEMPLATE_NAME BLYNK_DEVICE_NAME
#endif

#if !defined(BLYNK_TEMPLATE_ID) || !defined(BLYNK_TEMPLATE_NAME)
#error "Please specify your BLYNK_TEMPLATE_ID and BLYNK_TEMPLATE_NAME"
#endif

#if defined(BLYNK_AUTH_TOKEN)
#error "BLYNK_AUTH_TOKEN is assigned automatically when using Blynk.Edgent, please remove it from the configuration"
#endif

BlynkTimer edgentTimer;

#include "BlynkState.h"
#include "ConfigStore.h"
#include "ResetButton.h"
#include "ConfigMode.h"
#include "Indicator.h"
#include "OTA.h"
#include "Console.h"


inline void BlynkState::set(State m) {
  if (state != m && m < MODE_MAX_VALUE) {
    // DEBUG_PRINT(String(StateStr[state]) + " => " + StateStr[m]);
    state = m;

    // You can put your state handling here,
    // i.e. implement custom indication
  }
}

void printDeviceBanner() {
#ifdef BLYNK_PRINT
  Blynk.printBanner();
  BLYNK_PRINT.println("----------------------------------------------------");
  BLYNK_PRINT.print(" Device:    ");
  BLYNK_PRINT.println(getWiFiName());
  BLYNK_PRINT.print(" Firmware:  ");
  BLYNK_PRINT.println(BLYNK_FIRMWARE_VERSION " (build " __DATE__ " " __TIME__ ")");
  if (configStore.getFlag(CONFIG_FLAG_VALID)) {
    BLYNK_PRINT.print(" Token:     ");
    BLYNK_PRINT.println(String(configStore.cloudToken).substring(0, 4) + " - •••• - •••• - ••••");
  }
  BLYNK_PRINT.print(" Platform:  ");
  BLYNK_PRINT.println(String(BLYNK_INFO_DEVICE) + " @ " + ESP.getCpuFreqMHz() + "MHz");
  BLYNK_PRINT.print(" Boot ver:  ");
  BLYNK_PRINT.println(ESP.getBootVersion());
  BLYNK_PRINT.print(" SDK:       ");
  BLYNK_PRINT.println(ESP.getSdkVersion());
  BLYNK_PRINT.print(" ESP Core:  ");
  BLYNK_PRINT.println(ESP.getCoreVersion());
  BLYNK_PRINT.print(" Flash:     ");
  BLYNK_PRINT.println(String(ESP.getFlashChipSize() / 1024) + "K");
  BLYNK_PRINT.print(" Free mem:  ");
  BLYNK_PRINT.println(ESP.getFreeHeap());
  BLYNK_PRINT.println("----------------------------------------------------");
#endif
}

void runBlynkWithChecks() {
  Blynk.run();
  if (BlynkState::get() == MODE_RUNNING) {
    if (!Blynk.connected()) {
      if (WiFi.status() == WL_CONNECTED) {
        BlynkState::set(MODE_CONNECTING_CLOUD);
      } else {
        BlynkState::set(MODE_CONNECTING_NET);
      }
    }
  }
}

class Edgent {

public:
  void begin() {

#ifdef BLYNK_FS
    BLYNK_FS.begin();
#endif

    indicator_init();
    button_init();
    config_init();
    printDeviceBanner();
    console_init();

    if (configStore.getFlag(CONFIG_FLAG_VALID)) {
      BlynkState::set(MODE_CONNECTING_NET);
    } else if (config_load_blnkopt()) {
      // DEBUG_PRINT("Firmware is preprovisioned");
      BlynkState::set(MODE_CONNECTING_NET);
    } else {
      BlynkState::set(MODE_WAIT_CONFIG);
    }

    if (!String(BLYNK_TEMPLATE_ID).startsWith("TMPL") || !strlen(BLYNK_TEMPLATE_NAME)) {
      // DEBUG_PRINT("Invalid configuration of TEMPLATE_ID / TEMPLATE_NAME");
      while (true) { delay(100); }
    }
  }

  void run() {
    app_loop();
    switch (BlynkState::get()) {
      case MODE_WAIT_CONFIG:
      case MODE_CONFIGURING: enterConfigMode(); break;
      case MODE_CONNECTING_NET: enterConnectNet(); break;
      case MODE_CONNECTING_CLOUD: enterConnectCloud(); break;
      case MODE_RUNNING: runBlynkWithChecks(); break;
      case MODE_OTA_UPGRADE: enterOTA(); break;
      case MODE_SWITCH_TO_STA: enterSwitchToSTA(); break;
      case MODE_RESET_CONFIG: enterResetConfig(); break;
      default: enterError(); break;
    }
  }

} BlynkEdgent;

void app_loop() {
  edgentTimer.run();
  edgentConsole.run();
}
