#define ReleOn LOW
#define ReleOff HIGH
class Reles {
private:
  static const int qtdReles = 2;
  bool StatusReles[qtdReles];
  long RelePin[qtdReles];
public:
  Reles() {
    // Defina o Pino de Cada Relê
    this->RelePin[0] = D2;
    this->RelePin[1] = D3;
    for (int r = 0; r < this->qtdReles; r++) {
      this->StatusReles[r] = false;
      pinMode(this->RelePin[r], OUTPUT);
      digitalWrite(this->RelePin[r], ReleOff);
    }
  }
  void TurnOn(int pos) {
    if (!this->StatusReles[pos]) {
      this->StatusReles[pos] = true;
      digitalWrite(this->RelePin[pos], ReleOn);
    }
  }
  void TurnOff(int pos) {
    if (this->StatusReles[pos]) {
      this->StatusReles[pos] = false;
      digitalWrite(this->RelePin[pos], ReleOff);
    }
  }
  void Alternate(int pos) {
    if (this->StatusReles[pos]) {
      this->TurnOff(pos);
    } else {
      this->TurnOn(pos);
    }
  }
};
