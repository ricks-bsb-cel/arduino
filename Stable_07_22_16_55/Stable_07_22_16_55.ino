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

#define SERIAL_BAUD 9600   // Use este valor para ver também as msgs da própria placa
#define SERIAL1_BAUD 9600  // TXD1

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

// Leitora de temperatura e umidade do DHT 11
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const byte DHT_SENSOR_PIN = D3;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

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


class DHT11Sensor {
private:
  float temperature;
  float humidity;
public:
  bool Measure() {
    float t;
    float h;

    if (dht_sensor.measure(&t, &h)) {

      this->temperature = round(t * 10) / 10;
      this->humidity = round(h * 10) / 10;

      return (true);
    } else {
      return (false);
    }
  }
  float getTemperature() const {
    return this->temperature;
  }
  float getHumidity() const {
    return this->humidity;
  }

  String getTemperatureAsString() const {
    return String(this->temperature, 1) + "C";
  }
  String getHumidityAsString() const {
    return String(this->humidity, 1) + "%";
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
    u8g2.drawStr(this->col, this->pixelLinha[this->linha - 1], this->title);
    if (this->lastValue.length() > 0) {
      u8g2.drawStr(this->colValue, this->pixelLinha[this->linha - 1], this->lastValue);
    }
  }
  void ShowValue(String _id, String value) {
    if (this->id.equals(_id)) {
      u8g2.drawStr(this->colValue, this->pixelLinha[this->linha - 1], value);
    }
  }
};


// https://www.makerhero.com/blog/controlando-um-display-oled-com-a-biblioteca-ssd1306/
// https://adafruit.github.io/Adafruit_SSD1306/html/class_adafruit___s_s_d1306.html
class LcdOutput {
private:
  static const int qtdRows = 6;
  LcdRow rows[qtdRows];
  String topMessage = "SensorsNet.com";
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
  void Clear() {
    u8g2.clearBuffer();
    u8g2.drawFrame(1, 0, 127, 16);
    u8g2.drawStr(5, 12, this->topMessage);

    for (int i = 0; i < qtdRows; i++) {
      this->rows[i].Show();
    }

    u8g2.sendBuffer();
  }
  void Show() {
    u8g2.sendBuffer();
  }

  void Set(String id, String value) {
    for (int i = 0; i < qtdRows; i++) {
      this->rows[i].ShowValue(id, value);
    }
  };

  void Set(String id, String value, bool refresh) {
    for (int i = 0; i < qtdRows; i++) {
      this->rows[i].ShowValue(id, value);
    }
    this->Clear();
  };
};


class WiFiPayloadModel {
public:
  String Mac, LocalIP, Ssid, Hostname, Tipo, message;
  float Temperature, Humidity;
};


class WiFiNetwork {
private:
  LcdOutput* lcd;
  BuildInLed* led;

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
    WiFi.begin(defaultSsid, defaultPassword);

    int wait = 10;

    while (WiFi.status() != WL_CONNECTED && wait > 0) {
      this->led->Alternate();
      wait--;
      this->lcd->Set("netStatus", "Connecting " + String(wait), true);
      delay(1000);
    }

    this->isConnected = WiFi.status() == WL_CONNECTED;

    if (this->isConnected) {
      this->lcd->Set("netStatus", "Connected ", true);
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
      this->lcd->Set("msg", message, true);

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
        this->lcd->Set("netStatus", "Message sent", true);
      } else {
        this->lcd->Set("msg", message + "(Msg !Sent " + String(httpResponseCode) + ")", true);
        Serial.println(logUrl);
        Serial.println(strPayload);
      }

      http.end();
    }
  }

  void SendSensorData(WiFiPayloadModel payload) {
    if (WiFi.status() == WL_CONNECTED) {
      this->lcd->Set("netStatus", "Sending Data", true);

      WiFiClientSecure client;
      HTTPClient http;
      client.setInsecure();

      String strPayload = this->GetPayloadAsString(payload, "Sensor");

      http.begin(client, logUrl);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(strPayload);

      if (httpResponseCode > 0) {
        String response = http.getString();

        JsonDocument json;
        deserializeJson(json, response);

        if (json.containsKey("topMsg") && !json["topMsg"].isNull()) {
          this->lcd->SetTopMessage(json["topMsg"]);
        }

        if (json.containsKey("netResult") && !json["netResult"].isNull()) {
          this->lcd->Set("netStatus", json["netResult"]);
        }

        this->lcd->Clear();
        this->lcd->Show();
      } else {
        this->lcd->Set("netStatus", "Error " + String(httpResponseCode), true);
        Serial.println(logUrl);
        Serial.println(strPayload);
      }

      http.end();
    }
  }
};

// Coloque fora do loop o que deve ser inicializado apenas uma vez
BuildInLed buildLed;
LcdOutput lcd;
WiFiNetwork network(&lcd, &buildLed);
DHT11Sensor sensor;

void setup() {
  Serial.begin(SERIAL_BAUD);  // Usado para comunicação com a USB e debug print
  rs485Serial.begin(SERIAL_BAUD);

  network.Connect();
}

// Vamos testar -------------------------
static byte TempAndHumidity[] = {
  0x01,        // Endereço do dispositivo
  0x04,        // Código de função (ler Holding Registers)
  0x00, 0x01,  // Endereço inicial do registro (low byte e high byte)
  0x00, 0x02,  // Número de registros a serem lidos (low byte e high byte)
  0x20, 0x0B   // CRC (vamos calcular a seguir)
};

void loop() {
  buildLed.On();

  lcd.Clear();

  if (sensor.Measure()) {
    // WiFiPayloadModel payload = network.GetDefaultPayload("LocalSensor");

    lcd.Set("loctemp", sensor.getTemperatureAsString(), true);
    lcd.Set("lochumi", sensor.getHumidityAsString(), true);

    /*
    payload.Temperature = sensor.getTemperature();
    payload.Humidity = sensor.getHumidity();

    Serial.println();

    Serial.print("Local Temp: ");
    Serial.print(sensor.getTemperatureAsString());

    Serial.println();
    Serial.print("Local Humidity: ");
    Serial.print(sensor.getHumidityAsString());

    Serial.println();
    
    network.SendSensorData(payload);
    */
  }

  while (Serial.available()) {
    Serial.flush();
  }

  while (rs485Serial.available()) {
    rs485Serial.flush();
  }

  delay(1000);

  rs485Serial.write(TempAndHumidity, 8);  // Envia

  int wait = 30;

  while (!rs485Serial.available() && wait > 0) {
    wait--;
    delay(100);
  }

  int i = 0;
  byte receivedPayload[8];
  bool success = false;

  while (rs485Serial.available() && i < 8) {
    success = true;
    byte received = rs485Serial.read();
    receivedPayload[i] = received;
    i++;
  }

  if (success) {
    Util u;

    float tempResult = u.HexToFloat(receivedPayload[TEMP_HB], receivedPayload[TEMP_LB]);
    String sTempResult = String(tempResult, 1) + "C";

    float humidityResult = u.HexToFloat(receivedPayload[HUMIDITY_HB], receivedPayload[HUMIDITY_LB]);
    String sHumiResult = String(humidityResult, 1) + "%";

    lcd.Set("remtemp", sTempResult, true);
    lcd.Set("remhumi", sHumiResult, true);
  }
  // Vamos testar -------------------------

  buildLed.Off();

  delay(5000);  // O delay tem que ser o último comando do loop
}
