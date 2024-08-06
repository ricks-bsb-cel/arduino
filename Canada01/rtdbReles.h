struct RtdbData4Reles {
  bool rele1;
  bool rele2;
  bool rele3;
  bool rele4;
};

class rtdbReles {
private:
  WiFiNetwork* Net;
  LcdFreeWrite* Lcd;
  Util util;

  String DeviceId = util.MacToString(this->Net->GetMacId());

  int TimeOut = 10 * 1000;  // 10 segundos 
  int NextExecution = millis() + TimeOut;
  bool isOnExecution = false;

public:
  rtdbReles(WiFiNetwork* n, LcdFreeWrite* l) {
    this->Net = n;
    this->Lcd = l;
  }
  void Loop() {
    if (millis() > this->NextExecution && !this->isOnExecution && this->Net->isConnected()) {
      this->isOnExecution = true;

      // Next Execution
      this->NextExecution = millis() + this->TimeOut;
      this->isOnExecution = false;
    }
  }
};
