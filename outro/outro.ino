#include <U8g2lib.h>

U8G2 display; // Crie um objeto da biblioteca U8g2

void setup() {
  // Inicialize o display OLED
  // display.setFont(u8g2_font_5x7_tf); // Define a fonte a ser utilizada
  display.begin();
}

void loop() {
  // Escreva "Hello World!" na cor amarela na primeira linha
  display.drawStr(0, 0, "Hello World!"); // Escreve o texto na posição (0, 0)

  // Escreva "ESP8266" na cor azul na segunda linha
  display.drawStr(0, 12, "Funciona!"); // Escreve o texto na posição (0, 12)

  // Envie o buffer para o display
  display.sendBuffer();

  // Aguarda 1 segundo
  delay(1000);
}
