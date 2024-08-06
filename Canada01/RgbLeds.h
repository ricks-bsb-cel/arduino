#define LedRed D1
#define LedGreen D2
#define LedBlue D3

#define LedOff 0
#define LedOn 1
#define LedBlink 2

class RgbLeds {
private:
  const int OnTime = 500;

  bool BlueStatus = LedOff;
  bool RedStatus = LedOff;
  bool GreenStatus = LedOff;
public:
  RgbLeds() {
    pinMode(LedRed, OUTPUT);
    pinMode(LedGreen, OUTPUT);
    pinMode(LedBlue, OUTPUT);
    this->AllOff();
  };
  void AllOff() {
    digitalWrite(LedRed, LOW);
    digitalWrite(LedGreen, LOW);
    digitalWrite(LedBlue, LOW);
  }
  void Loop() {
    bool OnBlink = (millis() % this->OnTime) > (this->OnTime / 2);

    if (BlueStatus == LedOn || (BlueStatus == LedBlink && OnBlink)) {
      digitalWrite(LedBlue, HIGH);
    } else {
      digitalWrite(LedBlue, LOW);
    }

    if (GreenStatus == LedOn || (GreenStatus == LedBlink && OnBlink)) {
      digitalWrite(LedGreen, HIGH);
    } else {
      digitalWrite(LedGreen, LOW);
    }

    if (RedStatus == LedOn || (RedStatus == LedBlink && OnBlink)) {
      digitalWrite(LedRed, HIGH);
    } else {
      digitalWrite(LedRed, LOW);
    }
  }

  void Red(int status) {
    this->RedStatus = status;
  }

  void Blue(int status) {
    this->BlueStatus = status;
  }

  void Green(int status) {
    this->GreenStatus = status;
  }
};
