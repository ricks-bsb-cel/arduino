#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2(U8G2_R0, 14, 12, U8X8_PIN_NONE);

void setup(void) {
  u8g2.begin();
}

void loop(void) {
  u8g2.clearBuffer();

  u8g2.drawFrame(1, 0, 127, 16);

  // u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.setFont(u8g2_font_6x13_mf);
  u8g2.drawStr(5, 12, "Hello");

  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.drawStr(1, 26, "123456789012345678901");
  u8g2.drawStr(1, 38, "123456789012345678901");
  u8g2.drawStr(1, 50, "123456789012345678901");
  u8g2.drawStr(1, 62, "123456789012345678901");

  // u8g2.drawStr(1, 21, "Hello 12345678901234");
  // u8g2.drawStr(1, 31, "Hello 12345678901234");
  // u8g2.drawStr(1, 41, "Hello 12345678901234");

  u8g2.sendBuffer();

  delay(1000);
}