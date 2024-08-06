#include <Arduino.h>
#include <ArduinoJson.h>  // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <ArduinoJson.hpp>

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_SSD1306.h>
// #include <splash.h>

#include <Adafruit_GFX.h>
// #include <Adafruit_GrayOLED.h>
// #include <Adafruit_SPITFT.h>
// #include <Adafruit_SPITFT_Macros.h>

#include <gfxfont.h>


#include <DHT_Async.h>
// #include <ArduinoUniqueID.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>  // Include for HTTPS

#define LARGURA_OLED 128
#define ALTURA_OLED 64
#define RESET_OLED -1

Adafruit_SSD1306 display(LARGURA_OLED, ALTURA_OLED, &Wire, RESET_OLED);

// Configurações de envio para a internet
static const char* defaultSsid = "brisa-2154254";
static const char* defaultPassword = "syvxfcxh";
static String logUrl = "https://benditaia.web.app/eeb/sensors-net/module-receiver";

// Leitora de temperatura e umidade do DHT 11
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const byte DHT_SENSOR_PIN = D3;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

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
  int pixelLinha[6] = { 4, 16, 26, 36, 46, 56 };
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
    display.setCursor(this->col, this->pixelLinha[this->linha - 1]);
    display.print(this->title);
    if (this->lastValue.length() > 0) {
      display.setCursor(this->colValue, this->pixelLinha[this->linha - 1]);
      display.print(this->lastValue);
    }
  }
  void ShowValue(String _id, String value) {
    if (this->id.equals(_id)) {
      display.setCursor(this->colValue, this->pixelLinha[this->linha - 1]);
      display.print(value);
      this->lastValue = value;
    }
  }
};


// https://www.makerhero.com/blog/controlando-um-display-oled-com-a-biblioteca-ssd1306/
// https://adafruit.github.io/Adafruit_SSD1306/html/class_adafruit___s_s_d1306.html
class LcdOutput {
private:
  static const int qtdRows = 4;
  LcdRow rows[qtdRows];
  String topMessage = "SensorsNet.com";
public:
  void SetTopMessage(String msg) {
    this->topMessage = msg;
  }
  LcdOutput() {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.drawRect(0, 0, 127, 15, WHITE);

    const int valueColumn = 95;
    // ID, Row, Col, Texto, Valor
    this->rows[0] = LcdRow("netStatus", 2, 1, "Net:", 28);
    this->rows[1] = LcdRow("temp", 3, 1, "Loc:", 28);
    this->rows[2] = LcdRow("humi", 3, 62, "|", 72);
    this->rows[3] = LcdRow("msg", 4, 1, "Msg:", 28);
  }
  void Clear() {
    display.clearDisplay();
    display.drawRect(0, 0, 127, 15, WHITE);

    // Escreve a máscara
    display.setCursor(4, 4);
    display.print(this->topMessage);

    for (int i = 0; i < qtdRows; i++) {
      this->rows[i].Show();
    }
  }
  void Show() {
    display.display();
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
    this->Show();
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
      json["temp"] = round(payload.Temperature * 10) / 10;
      json["humidity"] = round(payload.Humidity * 10) / 10;
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

      http.end();

      if (httpResponseCode <= 0) {
        this->lcd->Set("msg", message + "(!Sent " + String(httpResponseCode) + ")", true);
      }
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
  network.Connect();
}

void loop() {
  buildLed.On();

  lcd.Clear();

  if (sensor.Measure()) {
    WiFiPayloadModel payload = network.GetDefaultPayload("LocalSensor");

    lcd.Set("temp", sensor.getTemperatureAsString(), true);
    lcd.Set("humi", sensor.getHumidityAsString(), true);

    payload.Temperature = sensor.getTemperature();
    payload.Humidity = sensor.getHumidity();

    network.SendSensorData(payload);
  }
  buildLed.Off();

  delay(15000);  // O delay tem que ser o último comando do loop
}
