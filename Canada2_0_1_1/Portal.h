const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;

IPAddress APIP(172, 0, 0, 1);  // Gateway
ESP8266WebServer webServer(80);

struct WifiNet {
  String SSID;
  int RSSI;
  String encryptionType;
};

class Portal {
private:
  String MacAddress = WiFi.macAddress();
  EepromStorage *Eeprom;
  LcdLog *lcd;
  Util util;
  bool IsAccessPointActive = false;
  String Title;
  String htmlSelectWiFi;

  String currentSSID;
  DNSServer dnsServer;

  bool UserWiFiIsConnected = false;
  bool ConnectedMessage = false;

  String formWifiName = "";
  String formWifiPassword = "";
  String formEmail = "";

  unsigned long RestartTimeout = 0;

  String htmlHeader(String SubTitle) {
    String CSS = "div,h1{padding:.5em}body,label{color:#333}small{font-size:0.5em}label,nav,nav b{display:block}input,select{width:100%}article{background:#f2f2f2;padding:1.3em}input{font-family:monospace}body{font-family:Century Gothic,sans-serif;font-size:18px;line-height:24px;margin:0;padding:0}h1{margin:.5em 0 0}input, select{padding:9px 10px;margin:8px 0;box-sizing:border-box;border-radius:10px;border:1px solid #555}label{font-style:italic;font-weight:700}nav{background:#06f;color:#fff;font-size:1.3em;padding:1em}nav b{font-size:1.5em;margin-bottom:.5em}";
    String result = "<!DOCTYPE html><head><title>" + this->Title + "</title><meta name=viewport content=\"width=device-width,initial-scale=1\"><style>"
                    + CSS + "</style><meta charset=\"UTF-8\"></head><body><nav><b>"
                    + this->Title + "</b> " + SubTitle + "<br/><small>" + this->currentSSID + "</small>";
    result += "</nav><div>";

    return result;
  }

  String JSSendForm() {
    String result = R"(
      <script>
        document.getElementById('regform').addEventListener('submit', function(event) {
          event.preventDefault(); // Impede o envio padrão do formulário

          const form = event.target;
          const formData = new FormData(form);
          const formJson = {};

          formData.forEach((value, key) => {
            formJson[key] = value;
          });

          const submitElement = document.getElementById('submit');
          const msgElement = document.getElementById('msgresult');

          submitElement.style.display = "none";
          msgElement.textContent = "Checking..."

          fetch('/reg', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json'
            },
            body: JSON.stringify(formJson)
          })
          .then(response => response.json())
          .then(data => {
            msgElement.textContent = data.message;
          })
          .catch((error) => {
            msgElement.textContent = error;
          })
          .finally(() => {
            submitElement.style.display = "";
          });
        });
      </script>
    )";
    return result;
  }

  String htmlFooter() {
    return "</div><div class=q><a>&#169; All rights reserved.</a></div></div>" + JSSendForm() + "</body></html>";
  }

  String htmlPageIndex() {
    String Body = "Please provide the necessary information to connect the module to your internet.";
    String htmlResult = htmlHeader("Module Configuration") + "<div>" + Body + "</ol></div><div><form id=regform><label>Select your WiFi Network:</label>"
                        + this->htmlSelectWiFi
                        + R"(<label>WiFi Password:</label><input type=text name=wifipass></input>)"
                        + R"(<input type=submit id=submit value="Save and Restart"></input></form>)"
                        + R"(<br/><strong id=msgresult></strong>)"
                        + htmlFooter();

    this->RestartTimeout = millis() + 180000L;  // 3 Minutes

    if (!ConnectedMessage) {
      ConnectedMessage = true;
      lcd->Log("Waiting for");
      lcd->Log("new Configuration...");
      lcd->Log("-----------------------");
    }

    return htmlResult;
  }

  String jsonRegister() {
    String payload = webServer.arg("plain");

    JsonDocument result;
    result["success"] = false;

    JsonDocument request;
    deserializeJson(request, payload);

    if (!request.containsKey("wifiname") || request["wifiname"].isNull() || request["wifiname"].as<String>().isEmpty()) {
      result["message"] = "Choose the Wi-Fi network you want to connect to.";
    } else if (!request.containsKey("wifipass") || request["wifipass"].isNull() || request["wifipass"].as<String>().isEmpty()) {
      result["message"] = "Please provide the Wi-Fi password.";
    } else {
      result["wifiSsid"] = request["wifiname"];
      result["wifiPass"] = request["wifipass"];

      // Salva a nova configuração de WiFi
      this->lcd->Log("Updating WiFi Setup");
      this->Eeprom->SaveData(String(result["wifiSsid"]), String(result["wifiPass"]));
      this->lcd->Log("WiFi Setup Updating");
      this->lcd->Log("Restarting...");
      delay(2000);
      util.Reboot();

      result["message"] = "WiFi Setup Updated. Restarting...";
    }

    String strResult;
    serializeJson(result, strResult);

    Serial.println(strResult);

    return strResult;
  }

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

  std::vector<WifiNet> GetWifiNetworks() {
    std::vector<WifiNet> result;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int numSsid = WiFi.scanNetworks();
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

  String GetWifiNetworksAsHtmlSelect() {
    std::vector<WifiNet> networks = this->GetWifiNetworks();

    String result = R"(<select name="wifiname">)";
    result += R"(<option value="invalid">Selecione</option>)";

    for (const WifiNet &net : networks) {
      result += R"(<option value=")" + net.SSID + R"(">)" + net.SSID + " (" + this->classifyRSSI(net.RSSI) + ")" + R"(</option>)";
    }

    result += R"(</select>)";

    return result;
  }

  void StartAccessPoint() {
    this->htmlSelectWiFi = this->GetWifiNetworksAsHtmlSelect();

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(this->currentSSID);  // Nome da Rede

    dnsServer.start(DNS_PORT, "*", APIP);  // DNS spoofing (Only for HTTP)

    webServer.on("/reg", [this]() {
      webServer.send(HTTP_CODE, "application/json", this->jsonRegister());
    });

    webServer.onNotFound([this]() {
      webServer.send(HTTP_CODE, "text/html", this->htmlPageIndex());
    });

    // Finaliza a habilitação do AccessPoint
    webServer.begin();

    this->IsAccessPointActive = true;

    // Se não houver conexão vai reiniciar em 2 minutos
    this->RestartTimeout = millis() + 120000L;
  }

public:
  Portal(LcdLog *l, EepromStorage *e, String title) {
    this->lcd = l;
    this->Eeprom = e;
    this->Title = title;
  }

  bool Active() {
    return this->IsAccessPointActive;
  }

  void Start() {
    String MacCode = util.MacToString(this->MacAddress.c_str());
    this->currentSSID = this->Title + "_" + MacCode;

    lcd->Log("-----------------------");
    lcd->Log("Please, connect to:");
    lcd->Log(this->currentSSID);
    lcd->Log("and setup my WiFi");
    lcd->Log("-----------------------");

    lcd->SetTopMessage(this->currentSSID);

    this->StartAccessPoint();
  }

  void Loop() {
    if (this->IsAccessPointActive) {
      dnsServer.processNextRequest();
      webServer.handleClient();

      if (this->RestartTimeout > 0 && millis() > this->RestartTimeout) {
        lcd->Log("Rebooting");
        delay(2000);
        this->util.Reboot();
      }
    }
  }
};