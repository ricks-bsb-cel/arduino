#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>

#include <gfxfont.h>

#include <Arduino.h>
#include <ArduinoJson.h>  // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <ArduinoJson.hpp>

// #include <U8g2lib.h>
#include <DHT_Async.h>
#include <ArduinoUniqueID.h>

#include <WiFiClientSecure.h>  // Include for HTTPS
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// #include <WiFiClient.h>

// static const unsigned long SERIAL_BAUD_RATE = 9600;
#define SERIAL_BAUD_RATE 9600
#define LARGURA_OLED 128
#define ALTURA_OLED 64
#define RESET_OLED -1

Adafruit_SSD1306 display(LARGURA_OLED, ALTURA_OLED, &Wire, RESET_OLED);

// Configurações de envio para a internet
static const char* defaultSsid = "brisa-2154254";
static const char* defaultPassword = "syvxfcxh";

static String logUrl = "https://benditaia.web.app/eeb/sensors-net/log";

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
    return String(this->temperature, 1) + "c";
  }
  String getHumidityAsString() const {
    return String(this->humidity, 1) + "%";
  }
};


// Utilidades
class Utils {
public:
  // Retorna o UniqueID da placa ou processador
  String getUniqueID() {
    Serial.println("getUniqueID");
    String result = "";
    for (size_t i = 0; i < 8; i++) {
      if (UniqueID8[i] < 0x10) {
        result += "0";
      }
      result += String(UniqueID8[i], HEX);
      if (i < 7) {
        result += " ";
      }
    }
    return result;
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
// Pinos: SDA => D2 (GPIO 4)
//        SCL => D1 (GPIO 5)
static const int qtdRows = 5;
class LcdOutput {
private:
  LcdRow rows[qtdRows];
public:
  LcdOutput() {
    Serial.println("Lcd Setup Start...");

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.drawRect(0, 0, 127, 15, WHITE);

    const int valueColumn = 95;

    this->rows[0] = LcdRow("top", 1, 4, "SensorsNet.com", 0);
    this->rows[1] = LcdRow("netStatus", 2, 1, "WiFi:", 31);
    this->rows[2] = LcdRow("lastCom", 3, 1, "LCom:", 31);
    this->rows[3] = LcdRow("temp", 4, 1, "Loc T:", 52);
    this->rows[4] = LcdRow("humi", 4, 82, "H:", 92);

    Serial.println("Lcd Setup Ready...");
  }
  void Clear() {
    display.clearDisplay();
    display.drawRect(0, 0, 127, 15, WHITE);

    // Já escreve a máscara
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
  String DeviceID, Mac, LocalIP, GatewayIP, Channel, Ssid, Hostname;
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

    this->lcd->Set("netStatus", "Disconnected", true);
  }

  WiFiPayloadModel GetDefaultPayload() {
    WiFiPayloadModel payload;
    Utils util;

    payload.DeviceID = util.getUniqueID();
    payload.Ssid = WiFi.SSID();
    payload.Hostname = WiFi.getHostname();
    payload.LocalIP = WiFi.localIP().toString();
    payload.GatewayIP = WiFi.gatewayIP().toString();
    payload.Channel = WiFi.channel();
    payload.Mac = WiFi.macAddress();
    payload.Temperature = NAN;
    payload.Humidity = NAN;

    return payload;
  };

  String GetLogPayload(String message) {
    JsonDocument json;

    json["message"] = message;

    String result;
    serializeJson(json, result);

    return result;
  }

  String GetPayloadAsString(WiFiPayloadModel payload) {
    JsonDocument json;

    json["deviceId"] = payload.DeviceID;
    json["ssId"] = payload.Ssid;
    json["hostName"] = payload.Hostname;
    json["localIp"] = payload.LocalIP;
    json["gatewayIp"] = payload.GatewayIP;
    json["channel"] = payload.Channel;
    json["mac"] = payload.Mac;

    if (!isnan(payload.Temperature) && !isnan(payload.Humidity)) {
      json["temp"] = round(payload.Temperature * 10) / 10;
      json["humidity"] = round(payload.Humidity * 10) / 10;
    }

    String result;
    serializeJson(json, result);

    return result;
  }

  void UpdateLastContact(String text) {
    JsonDocument json;
    deserializeJson(json, text);

    this->lcd->Set("netStatus", "Connected", true);

    if (json.containsKey("momment") && !json["momment"].isNull()) {
      this->lcd->Set("lastCom", json["momment"], true);
    }
  }

  void Connect() {
    Serial.print("Connection to WiFi ");
    Serial.println(defaultSsid);
    Serial.println();

    WiFi.begin(defaultSsid, defaultPassword);

    int wait = 10;
    this->lcd->Set("netStatus", "Connecting " + String(wait), true);

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

  void Log(String message) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      HTTPClient http;

      client.setInsecure();

      String strPayload = this->GetLogPayload(message);

      http.begin(client, logUrl);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(strPayload);

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        this->UpdateLastContact(payload);
        Serial.println(payload);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    }
  }

  void UploadPayload(WiFiPayloadModel payload) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      HTTPClient http;

      client.setInsecure();

      String strPayload = this->GetPayloadAsString(payload);

      http.begin(client, logUrl);
      http.addHeader("Content-Type", "application/json");

      Serial.println(logUrl);
      Serial.println(strPayload);

      int httpResponseCode = http.POST(strPayload);

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        this->UpdateLastContact(payload);
        Serial.println(payload);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    }
  }
};

// Coloque fora do loop o que deve ser inicializado apenas uma vez
BuildInLed buildLed;
LcdOutput lcd;
// WiFiNetwork network(&lcd, &buildLed);
// DHT11Sensor sensor;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("Setup Start...");

  // network.Connect();
}

void loop() {
  Serial.println("loop");

  buildLed.Alternate();
  lcd.Clear();

  /*

  if (sensor.Measure()) {
    WiFiPayloadModel payload = network.GetDefaultPayload();

    lcd.Set("temp", sensor.getTemperatureAsString(), true);
    lcd.Set("humi", sensor.getHumidityAsString(), true);

    payload.Temperature = sensor.getTemperature();
    payload.Humidity = sensor.getHumidity();

    network.UploadPayload(payload);
  }

  buildLed.Off();
  */

  delay(5000);  // O delay tem que ser o último comando do loop
}
