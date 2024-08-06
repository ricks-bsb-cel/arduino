static String endPoint = "http://webhook.site/5f0331a3-9f92-42b0-9334-2f30430d3659";

/*
// openssl x509 -in ca_cert.pem -noout -dates
// openssl s_client -showcerts -connect benditaia.web.app:443 </dev/null 2>/dev/null | openssl x509 -outform PEM > ca_cert.pem
// openssl s_client -connect benditaia.web.app:443 < /dev/null | openssl x509 -noout -fingerprint -sha1
const char IRG_Root_X1[] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIFIzCCBAugAwIBAgIQLB9SZXmhunsKpHkdCr/hqTANBgkqhkiG9w0BAQsFADA7
MQswCQYDVQQGEwJVUzEeMBwGA1UEChMVR29vZ2xlIFRydXN0IFNlcnZpY2VzMQww
CgYDVQQDEwNXUjQwHhcNMjQwNzI2MTkzMDI4WhcNMjQxMDI0MTkzMDI3WjASMRAw
DgYDVQQDEwd3ZWIuYXBwMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA
rKp9e7hsF9AUHZ/9dN1TNr5Lr8cycK+RIVz4EU0wOOCG4xD5JyZ1qmQp7eDBlnnN
wkl9+EcfwUGXcEwR0u9YQXG12pSo3asE1vG/O7tVwDjPwKN1Nd/DcNn56w8phNoo
GNTkDopBtPDpA9i/x1n4fPdxWfATHfGn/F6oUAbDWxy+4inZAJ4CCJY7eLHOhmjE
1LB66wCk8vXgfAkiPrResuNMe4B9Ei4nA69D7/TZLklv2s97lEyRz3DuoJhRL6Tw
7hLrGVcuk9nDiPwch9K0fRCXq11lqXc5OTu7/rlgPH/U+m4AUkKF0tTUCPBX5IYN
HrNXzOjKESNLj+4roqxmXwIDAQABo4ICSjCCAkYwDgYDVR0PAQH/BAQDAgWgMBMG
A1UdJQQMMAoGCCsGAQUFBwMBMAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFOYoJwcM
ARk9Y9a74eI82bYD0OOFMB8GA1UdIwQYMBaAFJvIEbw9qja5MYxOj0TVVzIvw8Bh
MF4GCCsGAQUFBwEBBFIwUDAnBggrBgEFBQcwAYYbaHR0cDovL28ucGtpLmdvb2cv
cy93cjQvTEI4MCUGCCsGAQUFBzAChhlodHRwOi8vaS5wa2kuZ29vZy93cjQuY3J0
MB0GA1UdEQQWMBSCB3dlYi5hcHCCCSoud2ViLmFwcDATBgNVHSAEDDAKMAgGBmeB
DAECATA2BgNVHR8ELzAtMCugKaAnhiVodHRwOi8vYy5wa2kuZ29vZy93cjQvQnNo
Y0NxZzJhZWcuY3JsMIIBAwYKKwYBBAHWeQIEAgSB9ASB8QDvAHYAdv+IPwq2+5VR
wmHM9Ye6NLSkzbsp3GhCCp/mZ0xaOnQAAAGQ8Lz6MQAABAMARzBFAiEA+E6tpx3G
E0NApYNVSYX4EWzTLwbGejFYyWOsfX2eNecCIFY67MDqh0klokVrMhyvSu7zspGk
A8MCvS1qqXgAZH0+AHUAPxdLT9ciR1iUHWUchL4NEu2QN38fhWrrwb8ohez4ZG4A
AAGQ8Lz6EQAABAMARjBEAiAimUZMB1c1i+tYwnSVHz+YsxpJyyY30+LhOPTQBx0V
rAIgA2NwdIq7ngYINvewL7TEVzy0DL5Ja5UMlvIA8ZiVf2EwDQYJKoZIhvcNAQEL
BQADggEBAIW+QXTdaVJ7KxRgE143tZQxt6DQ8xsRuefiFTL1fuKWInXMhaZTpXkY
dceNMlyGbSD5KM+2iff6+WJ98021L02qTykkZf0G306KY7bfniI6FWaz633KHu1y
jaS9EuqAwWFTz5WtKXnLGkILHsfAQ9InQO6mDrZNNvdbD86kNTiLOKuj3HLC0Rc6
a5fMdwZBXIlEup20h0OLJNt/oN8xdpgkyIzxAyHvBW2N19lkZgoHgqSJNXth/jvf
CVsf4ILEWQqm7ZbnN3mHYZrE37pe/LyUwkU2KvO2SVsLnlNX8CGLeYLxktXikFeP
7k5/Q8+VHmqn++elarQFqMbjCxgnoI0=
-----END CERTIFICATE-----
)CERT";

X509List cert(IRG_Root_X1);
*/

struct WifiNet {
  String SSID;
  int RSSI;
  String encryptionType;
};

class WiFiPayloadModel {
public:
  String Mac, LocalIP, Ssid, Hostname, Tipo, message;
};

  void myTimerEvent() {
    // You can send any value at any time.
    // Please don't send more that 10 values per second.
    Blynk.virtualWrite(V2, millis() / 1000);
  }

class WiFiNetwork {
private:
  BlynkTimer *timer;
  BuildInLed buildLed;

  String encryptionTypeToString(uint8_t encryptionType) {
    switch (encryptionType) {
      case ENC_TYPE_WEP: return "WEP";
      case ENC_TYPE_TKIP: return "WPA/WPA2 TKIP";
      case ENC_TYPE_CCMP: return "WPA2 CCMP";
      case ENC_TYPE_NONE: return "Open";
      case ENC_TYPE_AUTO: return "Auto";
      default: return "Unknown";
    }
  };

  String classifyRSSI(int RSSI) {
    if (RSSI >= -50) return "Sinal Excelente";
    if (RSSI >= -60) return "Sinal Muito Bom";
    if (RSSI >= -70) return "Sinal Bom";
    if (RSSI >= -80) return "Sinal Fraco";
    return "Sinal Muito Fraco";
  }

public:
  WiFiNetwork(BlynkTimer *t) {
    this->timer = t;
  }
  std::vector<WifiNet> GetWifiNetworks() {
    std::vector<WifiNet> result;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int numSsid = WiFi.scanNetworks();
    delay(100);

    delay(5000);
    for (int i = 0; i < numSsid; i++) {
      WifiNet found = {
        WiFi.SSID(i),
        WiFi.RSSI(i),
        encryptionTypeToString(WiFi.encryptionType(i))
      };
      result.push_back(found);
    }

    // Ordena as redes Wi-Fi pela força do sinal (RSSI) do mais forte para o mais fraco
    std::sort(result.begin(), result.end(), [](const WifiNet &a, const WifiNet &b) {
      return a.RSSI > b.RSSI;
    });

    return result;
  }

  bool ConnectWiFi(String ssid, String password) {
    const char *id = ssid.c_str();
    const char *pass = password.c_str();

    Blynk.begin(BLYNK_AUTH_TOKEN, id, pass);
    // Setup a function to be called every second
    this->timer->setInterval(1000L, this->myTimerEvent);

    /*
    WiFi.disconnect();
    delay(100);

    WiFi.mode(WIFI_STA);
    WiFi.begin(id, pass);

    int tries = 60;  // Tentar conectar por um tempo determinado

    while (WiFi.status() != WL_CONNECTED && tries-- > 0) {
      delay(250);
      buildLed.Alternate();
    }
    */

    bool isConnected = (WiFi.status() == WL_CONNECTED);

    /*
    if (isConnected) {
      // Set time via NTP, as required for x.509 validation
      configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

      Serial.print("Waiting for NTP time sync: ");
      time_t now = time(nullptr);
      while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
      }
      Serial.println("");
      struct tm timeinfo;
      gmtime_r(&now, &timeinfo);
      Serial.print("Current time: ");
      Serial.print(asctime(&timeinfo));
    }
    */

    return isConnected;
  }

  String GetMacId() {
    return WiFi.macAddress();
  }

  bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
  }

  WiFiPayloadModel GetDefaultPayload(String Tipo) {
    WiFiPayloadModel payload;

    payload.Ssid = WiFi.SSID();
    payload.Hostname = WiFi.getHostname();
    payload.LocalIP = WiFi.localIP().toString();
    payload.Mac = WiFi.macAddress();
    payload.Tipo = Tipo;

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

    String result;
    serializeJson(json, result);

    return result;
  }

  String GetWifiNetworksAsHtmlSelect(String currentSsid = "invalid") {
    std::vector<WifiNet> networks = GetWifiNetworks();

    String result = R"(<select name="wifiname">)";
    result += R"(<option value="invalid">Selecione</option>)";

    for (const WifiNet &net : networks) {
      if (net.SSID == currentSsid) {
        result += R"(<option value=")" + net.SSID + R"(" selected>)" + net.SSID + " (" + this->classifyRSSI(net.RSSI) + ")" + R"(</option>)";
      } else {
        result += R"(<option value=")" + net.SSID + R"(">)" + net.SSID + " (" + this->classifyRSSI(net.RSSI) + ")" + R"(</option>)";
      }
    }

    result += R"(</select>)";

    return result;
  }

  String SendLog(String message) {
    String result = "!connected";

    if (WiFi.status() == WL_CONNECTED) {
      String url = endPoint;
      Serial.println(url);

      // Prepare PayLoad
      WiFiPayloadModel payload = this->GetDefaultPayload("Message");
      payload.message = message;
      String strPayload = this->GetPayloadAsString(payload, "Message");

      Serial.println(strPayload);

      Serial.println("WiFiClientSecure");
      WiFiClient client;

      // Serial.println("setInsecure");
      // client.setInsecure(); <== ISSO DÁ ERRO!

      Serial.println("http");
      HTTPClient http;

      Serial.println("begin");
      http.begin(client, url);
      // http.addHeader("Content-Type", "application/json");

      Serial.println("POST");
      int httpResponseCode = http.POST(strPayload);

      Serial.println(httpResponseCode);

      if (httpResponseCode > 0) {
        Serial.println(http.getString());
      }

      http.end();
    } else {
      Serial.println("[HTTPS] Unable to connect\n");
    }
    return result;
  }
};
