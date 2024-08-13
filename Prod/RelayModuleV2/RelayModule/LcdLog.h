#include <U8g2lib.h>

#define LARGURA_OLED 128
#define ALTURA_OLED 64
#define RESET_OLED -1

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 14, 12, U8X8_PIN_NONE);  // Configurações do Display OnBoard (FUNCIONA OK)
// U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, D1, D2, U8X8_PIN_NONE); // Com display externo conectado em D1/D2

// https://www.makerhero.com/blog/controlando-um-display-oled-com-a-biblioteca-ssd1306/
// https://adafruit.github.io/Adafruit_SSD1306/html/class_adafruit___s_s_d1306.html
// https://github.com/olikraus/u8g2/wiki/u8g2reference#setflipmode
class LcdLog {
private:
  bool lcdEnabled = false;

  const static int lines = 5;
  const static int lineHeight = 10;

  int pixelRow[lines];
  String Rows[lines];

  String topMessage = "Initializing...";
  int currentRow = -1;  // Será incrementado na primeira escrita.
public:
  // Constructor
  LcdLog(String tm, bool enabled = true) {
    this->topMessage = tm;
    this->lcdEnabled = enabled;

    if (this->lcdEnabled) {

      int firstLine = 23;

      for (int i = 0; i < lines; i++) {
        pixelRow[i] = firstLine;
        Rows[i] = "";
        firstLine += lineHeight;
      }

      u8g2.begin();  // Inicialização do display
      // u8g2.setFlipMode(1);

      u8g2.clearBuffer();
      u8g2.drawFrame(1, 0, 127, 16);

      u8g2.setFont(u8g2_font_6x13_mf);
      u8g2.drawStr(5, 12, this->topMessage.c_str());

      u8g2.sendBuffer();
    }
  }

  void SetTopMessage(String msg) {
    this->topMessage = msg;
    this->Refresh();
  }

  void Write(String text, bool override = false) {
    if (this->lcdEnabled) {
      if (!override) {
        if (currentRow < 4) {
          this->currentRow++;
        } else {
          for (int i = 0; i < 4; i++) {
            this->Rows[i] = this->Rows[i + 1];
          }
        }
      }
      this->Rows[this->currentRow] = text.substring(0, 21);

      this->Refresh();
    } else {
      Serial.println(text);
    }
  }

  void ShowOnSameLine(String text) {
    this->Write(text, true);
  }

  void Log(String text) {
    this->Write(text);
  }

  void Log(int value) {
    this->Write(String(value));
  }

  void Refresh() {
    if (this->lcdEnabled) {
      u8g2.clearBuffer();
      u8g2.drawFrame(1, 0, 127, 16);

      u8g2.setFont(u8g2_font_8x13_mf);
      u8g2.drawStr(5, 12, this->topMessage.c_str());

      u8g2.setFont(u8g2_font_profont11_tf);  //  u8g2_font_mozart_nbp_t_all

      for (int i = 0; i < 5; i++) {
        u8g2.drawStr(1, this->pixelRow[i], this->Rows[i].c_str());
      }

      u8g2.sendBuffer();
    }
  }
};
