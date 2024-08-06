#include <Arduino.h>
#include <ArduinoJson.h>  // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <ArduinoJson.hpp>

#include <SoftwareSerial.h>

#include <SPI.h>
#include <Wire.h>

#include <U8g2lib.h>

#include <DHT_Async.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>  // Include for HTTPS

// Defina os pinos para a comunicação RS485
#define RS485_RX_PIN 12  // GPIO12 (D6)
#define RS485_TX_PIN 13  // GPIO13 (D7)

#define SERIAL_BAUD 74880  // Use este valor para ver também as msgs da própria placa
#define SOFTWARE_SERIAL_BAUD 9600
#define LARGURA_OLED 128
#define ALTURA_OLED 64
#define RESET_OLED -1

#define SLAVE_ADDRESS 0
#define FUNCTION_CODE 1
#define BYTE 2
#define TEMP_HB 3
#define TEMP_LB 4
#define HUMIDITY_HB 5
#define HUMIDITY_LB 6
#define CRC_HB 7
#define CRC_LB 8

// Configurações do Display OnBoard
U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2(U8G2_R0, 14, 12, U8X8_PIN_NONE);

// Configurações de envio para a internet
static const char* defaultSsid = "brisa-2154254";
static const char* defaultPassword = "syvxfcxh";
static String logUrl = "https://benditaia.web.app/eeb/sensors-net/module-receiver";


// Crie uma instância de SoftwareSerial
SoftwareSerial rs485Serial(RS485_RX_PIN, RS485_TX_PIN);

class Util {
public:
  float HexToFloat(byte firstValue, byte secondValue) {
    char hexString[5];
    sprintf(hexString, "%02X%02X", firstValue, secondValue);
    unsigned long decimalValue = strtoul(hexString, NULL, 16);
    return static_cast<float>(decimalValue) / 10.0;
  }
};


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


class WiFiPayloadModel {
public:
  String Mac, LocalIP, Ssid, Hostname, Tipo, message;
  float Temperature, Humidity;
};

/*
class WiFiNetwork {
private:
  LcdOutput* lcd;
  BuildInLed* led;

  void ShowResponse(String response) {
    JsonDocument json;
    deserializeJson(json, response);

    if (json.containsKey("topMsg") && !json["topMsg"].isNull()) {
      this->lcd->SetTopMessage(json["topMsg"]);
    }

    if (json.containsKey("netResult") && !json["netResult"].isNull()) {
      this->lcd->Set("netStatus", json["netResult"]);
    }
  }

public:
  bool isConnected = false;

  WiFiNetwork(LcdOutput* _lcd, BuildInLed* _led) {
    this->lcd = _lcd;
    this->led = _led;

    this->lcd->Set("netStatus", "Disconnected");
  }

  WiFiPayloadModel GetDefaultPayload(String Tipo) {
    WiFiPayloadModel payload;

    payload.Ssid = WiFi.SSID();
    payload.Hostname = WiFi.getHostname();
    payload.LocalIP = WiFi.localIP().toString();
    payload.Mac = WiFi.macAddress();
    payload.Temperature = NAN;
    payload.Humidity = NAN;
    payload.Tipo = Tipo;
    payload.message = NAN;

    return payload;
  };

  String GetPayloadAsString(WiFiPayloadModel payload, String msgType) {
    JsonDocument json;

    json["ssId"] = payload.Ssid;
    json["hostName"] = payload.Hostname;
    json["localIp"] = payload.LocalIP;
    json["mac"] = payload.Mac;
    json["type"] = msgType;

    if (payload.message && payload.message.length() > 0) {
      json["message"] = payload.message;
    }

    if (!isnan(payload.Temperature) && !isnan(payload.Humidity)) {
      json["loctemp"] = round(payload.Temperature * 10) / 10;
      json["lochumi"] = round(payload.Humidity * 10) / 10;
    }

    String result;
    serializeJson(json, result);

    return result;
  }

  void Connect() {
    Serial.println("Conectando...");
    WiFi.begin(defaultSsid, defaultPassword);

    int wait = 10;

    while (WiFi.status() != WL_CONNECTED && wait > 0) {
      this->led->Alternate();
      wait--;
      Serial.println("Connecting " + String(wait));
      this->lcd->Set("netStatus", "Connecting " + String(wait));
      delay(1000);
    }

    this->isConnected = WiFi.status() == WL_CONNECTED;

    if (this->isConnected) {
      Serial.println("Connected");
      this->lcd->Set("netStatus", "Connected");
    }
  }

  bool Conected() {
    return this->isConnected;
  }

  String MacAddress() {
    return WiFi.macAddress();
  }

  void SendLog(String message) {
    if (WiFi.status() == WL_CONNECTED) {
      this->lcd->Set("msg", message);
      this->lcd->Set("netStatus", "Sending Msg...");

      WiFiClientSecure client;
      HTTPClient http;
      client.setInsecure();

      WiFiPayloadModel payload = this->GetDefaultPayload("Message");
      payload.message = message;
      String strPayload = this->GetPayloadAsString(payload, "Message");

      http.begin(client, logUrl);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(strPayload);

      if (httpResponseCode > 0) {
        this->ShowResponse(http.getString());
      } else {
        this->lcd->Set("msg", message + "(Msg !Sent " + String(httpResponseCode) + ")");
        Serial.println(logUrl);
        Serial.println(strPayload);
      }

      http.end();
    }
  }

  void Log(String message) {
    this->SendLog(message);
  }

  void SendSensorData(WiFiPayloadModel payload) {
    if (WiFi.status() == WL_CONNECTED) {
      this->lcd->Set("netStatus", "Sending Data...");

      WiFiClientSecure client;
      HTTPClient http;
      client.setInsecure();

      String strPayload = this->GetPayloadAsString(payload, "Sensor");

      http.begin(client, logUrl);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(strPayload);

      if (httpResponseCode > 0) {
        this->ShowResponse(http.getString());
      } else {
        this->lcd->Set("netStatus", "Error " + String(httpResponseCode));
        Serial.println(logUrl);
        Serial.println(strPayload);
      }

      http.end();
    }
  }
};
*/

#define ReleOn LOW
#define ReleOff HIGH
class Reles {
private:
  static const int qtdReles = 4;
  bool StatusReles[qtdReles];
  long RelePin[qtdReles];
public:
  Reles() {
    // Defina o Pino de Cada Relê
    this->RelePin[0] = D1;
    this->RelePin[1] = D2;
    this->RelePin[2] = D3;
    this->RelePin[3] = D4;
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

// Coloque fora do loop o que deve ser inicializado apenas uma vez
BuildInLed buildLed;
// WiFiNetwork network(&lcd, &buildLed);
Reles rele;

void setup() {
  Serial.begin(SERIAL_BAUD);  // Usado para comunicação com a USB e debug print

  rele.TurnOn(0);
}


void loop() {
  buildLed.On();

  rele.Alternate(0);
  rele.Alternate(1);
  rele.Alternate(3);

  delay(1000);

  buildLed.Off();
}
