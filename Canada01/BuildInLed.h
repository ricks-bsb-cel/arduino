// Led integrado
class BuildInLed {
  bool isOn = false;
public:
  BuildInLed() {
    pinMode(BUILTIN_LED, OUTPUT);
  }
  void On() {
    digitalWrite(BUILTIN_LED, LOW);
  }
  void Off() {
    digitalWrite(BUILTIN_LED, HIGH);
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
