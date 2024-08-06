#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>  // Include for HTTPS

// Configurações de envio para a internet
static const char* defaultSsid = "brisa-2154254";
static const char* defaultPassword = "syvxfcxh";
static String logUrl = "https://benditaia.web.app/eeb/sensors-net/module-receiver";

class WiFiPayloadModel {
public:
  String Mac, LocalIP, Ssid, Hostname, Tipo, message;
  float Temperature, Humidity;
};

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


  /*

      /*
      WiFiClientSecure client;
      client.setTrustAnchors(&cert);
      HTTPClient https;

      Serial.print("[HTTPS] begin...\n");

      if (https.begin(client, url)) {
        Serial.print("[HTTPS] begin ok...\n");

        https.addHeader("Content-Type", "application/json");
        Serial.print("[HTTPS] Header ok...\n");

        int httpCode = https.POST(strPayload);

        if (httpCode > 0) {
          Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            Serial.println(payload);
          }
        } else {
          Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
        */
  */

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