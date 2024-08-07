#include <Wire.h>

class Utils {
public:
  float HexToFloat(byte firstValue, byte secondValue) {
    char hexString[5];
    sprintf(hexString, "%02X%02X", firstValue, secondValue);
    unsigned long decimalValue = strtoul(hexString, NULL, 16);
    return static_cast<float>(decimalValue) / 10.0;
  }

  char* stringToCharArray(const String& str) {
    char* charArray = new char[str.length() + 1];
    std::strcpy(charArray, str.c_str());
    return charArray;
  }

  void freeCharArray(char* charArray) {
    delete[] charArray;
  }

  void Reboot() {
    ESP.restart();
    for (;;) {}
  }

  String MacToString(const String& macAddress) {
    // Definindo uma tabela de códigos para conversão
    const char Code[16] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P' };

    std::vector<int> macDecimal(6, 0);
    std::stringstream ss(macAddress.c_str());
    std::string segment;
    String result = "";

    // Divide a string do endereço MAC em segmentos usando ':' como delimitador
    while (std::getline(ss, segment, ':')) {
      int pos = std::stoi(segment, nullptr, 16);
      char letter = Code[pos % 16];
      result += letter;
    }

    return result;
  }

};