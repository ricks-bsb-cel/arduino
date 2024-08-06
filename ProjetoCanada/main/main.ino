#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>
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

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

static const int SERIAL_BAUD_RATE = 9600;

// Configurações do Display
#define LARGURA_OLED 128
#define ALTURA_OLED 64
#define RESET_OLED -1
#define SCREEN_SIZE 0x3D  // 0x3D for 128x64
// #define SCREEN_SIZE 0x3C // 0x3C for 128x32

Adafruit_SSD1306 display(LARGURA_OLED, ALTURA_OLED, &Wire, RESET_OLED);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

// Configurações de envio para a internet
static const char* defaultSsid = "brisa-2154254";
static const char* defaultPassword = "syvxfcxh";
static String serverName = "http://webhook.site/3faf6dbd-6aa3-4465-9dfb-89aed9521531";

// Leitora de temperatura e umidade do DHT 11
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const byte DHT_SENSOR_PIN = D3;  // GPIO0
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
    return String(this->temperature, 1) + " c";
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


class LcdOutput {
  // https://www.makerhero.com/blog/controlando-um-display-oled-com-a-biblioteca-ssd1306/
  // https://adafruit.github.io/Adafruit_SSD1306/html/class_adafruit___s_s_d1306.html
private:
  const int qtdRows = 6;
  LcdRow rows[6];
public:
  LcdOutput() {
    Serial.println("LcdOutput constructor");
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0, true)) {
      Serial.println(("***** SSD1306 allocation failed *****"));
    } else {
      Serial.println(("***** SSD1306 allocation ok *****"));
    }

    // u8g2_for_adafruit_gfx.begin(display);

    /*
    if (!display.begin(SSD1306_, OLED_RESET)) {
      Serial.println(("***** SSD1306 allocation failed *****"));
    }
    */

    display.clearDisplay();
    // display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    // display.drawRect(0, 0, 127, 15, WHITE);

    const int valueColumn = 95;

    this->rows[0] = LcdRow("top", 1, 4, "SensorsNet.com", 0);
    this->rows[1] = LcdRow("mac", 2, 1, "Mac:", 25);
    this->rows[2] = LcdRow("netStatus", 3, 1, "WiFi:", 31);
    this->rows[3] = LcdRow("temp", 4, 1, " Temp:", 40);
    this->rows[4] = LcdRow("humi", 5, 1, "Humid:", 40);
  }
  void Clear() {
    display.clearDisplay();
    // display.drawRect(0, 0, 127, 15, WHITE);

    // Já escreve a máscara
    for (int i = 0; i < this->qtdRows; i++) {
      this->rows[i].Show();
    }
  }
  void Show() {
    display.display();
  }
  void Set(String id, String value) {
    for (int i = 0; i < this->qtdRows; i++) {
      this->rows[i].ShowValue(id, value);
    }
  };
  void Set(String id, String value, bool refresh) {
    for (int i = 0; i < this->qtdRows; i++) {
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

    this->lcd->Set("netStatus", "Disconnected");
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

  void Connect() {
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

  void UploadPayload(WiFiPayloadModel payload) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      String strPayload = this->GetPayloadAsString(payload);

      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(strPayload);

      if (httpResponseCode > 0) {
        String payload = http.getString();
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

  // lcd.Set("mac", network.MacAddress());
  // network.Connect();
}

void loop() {
  buildLed.On();

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
  */

  buildLed.Off();

  delay(1000);  // O delay tem que ser o último comando do loop
}
