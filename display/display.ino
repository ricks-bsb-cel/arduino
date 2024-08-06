#include <ArduinoJson.h> // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <ArduinoJson.hpp>

#include <Wire.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <DHT_Async.h>
#include <ArduinoUniqueID.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const int LINHA_1 = 10;
const int LINHA_2 = 26;
const int LINHA_3 = 41;
const int LINHA_4 = 56;

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const byte DHT_SENSOR_PIN = D5;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2(U8G2_R0,/*clock=*/14,/*data=*/12,U8X8_PIN_NONE);

// Configurações de envio para a internet
const char* ssid = "brisa-2154254";
const char* password = "syvxfcxh";
String serverName = "http://webhook.site/cc4375d8-19ba-4214-be2b-95a9acac1232";

class DHT11Sensor{
  private:
    float temperature;
    float humidity;

  public:
    bool Get(){
      float t;
      float h;

      if (dht_sensor.measure(&t, &h)) {
          this->temperature = t;
          this->humidity = h;
          return (true);
      }else{
          return (false);
      }
    }
    float getTemperature() const{ return this->temperature; }
    float getHumidity() const{ return this->humidity; }

    String getTemperatureAsString() const{ return String(this->temperature); }
    String getHumidityAsString() const{ return String(this->humidity); }
};

class Console{
  public:
    void Write(String text){
      Serial.println(text);
    }
};

class BuildInLed{
  bool isOn = false;

  public:
    BuildInLed(){
      pinMode(BUILTIN_LED, OUTPUT);
    }
    void On(){
      digitalWrite(BUILTIN_LED, LOW);
    }
    void Off(){
      digitalWrite(BUILTIN_LED, HIGH);
    }
    void Alternate(){
      if(isOn){
        Off();
      }else{
        On();
      }
      isOn = !isOn;
    }
};


void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Conectando-se ao wifi...");

  int tentativas = 10;

  while(WiFi.status() != WL_CONNECTED && tentativas>0) {
    delay(500);
    Serial.print(".");
    tentativas--;
  }
  Serial.println();

  Serial.println(WiFi.status());
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.channel());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.macAddress());

  for (size_t i = 0; i < UniqueIDsize; i++)
	{
		if (UniqueID[i] < 0x10)
			Serial.print("0");
		Serial.print(UniqueID[i], HEX);
		Serial.print(" ");
	}
	Serial.println();

  // Inicialize o display OLED
  // display.setFont(u8g2_font_5x7_tf); // Define a fonte a ser utilizada

  u8g2.begin();
}

BuildInLed buildLed;

void loop() {
  Console console;
  DHT11Sensor DhtSensor;

  char temp[3];
  char humi[3];

  buildLed.On();

  if(DhtSensor.Get()){
    float t = DhtSensor.getTemperature();
    float h = DhtSensor.getHumidity();

    Serial.println(DhtSensor.getHumidityAsString());
    Serial.println(DhtSensor.getTemperatureAsString());

    // https://github.com/olikraus/u8g2/wiki
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(0,10,"Sensor DHT11");
    u8g2.drawStr(0,26,"Temperatura:");
    u8g2.drawStr(0,41,"  % Umidade:");

    u8g2.drawStr(90,26,DhtSensor.getTemperatureAsString().c_str());
    u8g2.drawStr(90,41,DhtSensor.getHumidityAsString().c_str());

    u8g2.sendBuffer();

    // Envio dos dados para a internet (não é para ficar aqui, mas é um teste)
    if(WiFi.status() == WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      JsonDocument payload;
      payload["foo"] = "bar";
      payload["temperature"] = t;
      payload["humidity"] = h;

      String strPayload;
      serializeJson(payload, strPayload);

      Serial.print("String Payload: ");
      Serial.println(strPayload);

      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json"); // Não Suporta JSON!
      // http.addHeader("Content-Length", String(payload.length()));

      int httpResponseCode = http.POST(strPayload);

      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }

  }

  buildLed.Off();

  delay(2500);
}


