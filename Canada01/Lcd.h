#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#define LARGURA_OLED 128
#define ALTURA_OLED 64
#define RESET_OLED -1

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 14, 12, U8X8_PIN_NONE); // Configurações do Display OnBoard (FUNCIONA OK)
// U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, D1, D2, U8X8_PIN_NONE); // Com display externo conectado em D1/D2

// Display LCD
class LcdRow {
private:
  int pixelLinha[6] = { 12, 26, 38, 50, 62 };
  String id, title, lastValue;
  int col, linha, colValue;
  bool found;
public:
  LcdRow() {
    this->found = false;
  };
  LcdRow(String i, int l, int c, String t, int cv) {
    this->found = true;
    this->id = i;
    this->linha = l;
    this->col = c;
    this->title = t;
    this->colValue = cv;
  };
  bool isId(String id) {
    return (this->id == id);
  }
  void Show() {
    u8g2.setFont(u8g2_font_t0_12_tr);
    u8g2.drawStr(this->col, this->pixelLinha[this->linha - 1], this->title.c_str());
    if (this->lastValue.length() > 0) {
      u8g2.drawStr(this->colValue, this->pixelLinha[this->linha - 1], this->lastValue.c_str());
    }
  }
  void ShowValue(String _id, String value) {
    u8g2.setFont(u8g2_font_t0_12_tr);
    if (this->id.equals(_id)) {
      this->lastValue = value;
      u8g2.drawStr(this->colValue, this->pixelLinha[this->linha - 1], value.c_str());
    }
  }
};


// https://www.makerhero.com/blog/controlando-um-display-oled-com-a-biblioteca-ssd1306/
// https://adafruit.github.io/Adafruit_SSD1306/html/class_adafruit___s_s_d1306.html
class LcdOutput {
private:
  static const int qtdRows = 6;
  LcdRow rows[qtdRows];
  String topMessage = " * sensornerds.ca *";
public:
  void SetTopMessage(String msg) {
    this->topMessage = msg;
  }
  LcdOutput() {
    // Inicialização do display
    u8g2.begin();

    u8g2.clearBuffer();
    u8g2.drawFrame(1, 0, 127, 16);

    u8g2.setFont(u8g2_font_6x13_mf);
    u8g2.drawStr(5, 12, "Starting...");
    u8g2.setFont(u8g2_font_t0_12_tr);

    u8g2.sendBuffer();

    const int valueColumn = 95;
    // ID, Row, Col, Texto, Valor
    this->rows[0] = LcdRow("netStatus", 2, 1, "Net:", 28);

    this->rows[1] = LcdRow("loctemp", 3, 1, "Loc:", 28);
    this->rows[2] = LcdRow("lochumi", 3, 62, "|", 72);

    this->rows[3] = LcdRow("remtemp", 4, 1, "Rem:", 28);
    this->rows[4] = LcdRow("remhumi", 4, 62, "|", 72);

    this->rows[5] = LcdRow("msg", 5, 1, "Msg:", 28);
  }
  void Refresh() {
    u8g2.clearBuffer();
    u8g2.drawFrame(1, 0, 127, 16);
    u8g2.drawStr(5, 12, this->topMessage.c_str());

    for (int i = 0; i < qtdRows; i++) {
      this->rows[i].Show();
    }

    u8g2.sendBuffer();
  }
  void Set(String id, String value) {
    for (int i = 0; i < qtdRows; i++) {
      this->rows[i].ShowValue(id, value);
    }
    this->Refresh();
  };
};
