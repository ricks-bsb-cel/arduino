class Util {
public:
  float HexToFloat(byte firstValue, byte secondValue) {
    char hexString[5];
    sprintf(hexString, "%02X%02X", firstValue, secondValue);
    unsigned long decimalValue = strtoul(hexString, NULL, 16);
    return static_cast<float>(decimalValue) / 10.0;
  }

  bool isValidEmail(const String& email) {
    // Verifica se há um '@' no email
    int atIndex = email.indexOf('@');
    if (atIndex == -1) {
      return false;
    }

    // Verifica se há um '.' após o '@'
    int dotIndex = email.indexOf('.', atIndex);
    if (dotIndex == -1) {
      return false;
    }

    // Verifica se não há espaços no email
    if (email.indexOf(' ') != -1) {
      return false;
    }

    // Verifica se '@' não é o primeiro ou último caractere
    if (atIndex == 0 || atIndex == email.length() - 1) {
      return false;
    }

    // Verifica se '.' não é o último caractere
    if (dotIndex == email.length() - 1) {
      return false;
    }

    return true;
  }

  char* stringToCharArray(const String& str) {
    char* charArray = new char[str.length() + 1];
    std::strcpy(charArray, str.c_str());
    return charArray;
  }

  void freeCharArray(char* charArray) {
    delete[] charArray;
  }

  String MacToString(const String& macAddress) {
    // Definindo uma tabela de códigos para conversão
    const char Code[16] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P' };

    std::vector<int> macDecimal(6, 0);         // Vetor para armazenar os valores decimais
    std::stringstream ss(macAddress.c_str());  // Stringstream para dividir a string do endereço MAC
    std::string segment;                       // String para armazenar cada segmento do endereço MAC
    String result = "";                        // String para armazenar o resultado final

    // Divide a string do endereço MAC em segmentos usando ':' como delimitador
    while (std::getline(ss, segment, ':')) {
      int pos = std::stoi(segment, nullptr, 16);  // Converte cada segmento de hexadecimal para decimal
      char letter = Code[pos % 16];               // Mapeia o valor decimal para um caractere
      result += letter;                           // Adiciona o caractere ao resultado
    }

    return result;  // Retorna a string resultante
  }
};