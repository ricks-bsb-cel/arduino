#define BLYNK_PRINT Serial  // Enables Serial Monitor

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL2iGv0-zOt"
#define BLYNK_TEMPLATE_NAME "Edgent Test Template"
#define BLYNK_AUTH_TOKEN "aiuy2kd1Hfoj2hJPGZF5KBcfI2nkcBem"

#define BLYNK_FIRMWARE_VERSION "0.1.2"

// Following includes are for Arduino Ethernet Shield (W5100)
// If you're using another shield, see Boards_* examples
#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>

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

void setup() {
  // See the connection status in Serial Monitor
  Serial.begin(74880);

  // Here your Arduino connects to the Blynk Cloud.
  Blynk.begin("brisa-2154254", "syvxfcxh", BLYNK_AUTH_TOKEN);

  timer.setInterval(5000L, BlinkTimer);  // 1 Minuto
}

void loop() {
  // All the Blynk Magic happens here...
  Blynk.run();

  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
