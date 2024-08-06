const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;

IPAddress APIP(172, 0, 0, 1);  // Gateway
ESP8266WebServer webServer(80);

class Portal {
private:
  LcdFreeWrite* lcd;
  WiFiNetwork* Net;
  Util util;

  String Title;
  String htmlSelectWiFi;

  String currentSSID;
  DNSServer dnsServer;

  bool UserWiFiIsConnected = false;

  String formWifiName = "";
  String formWifiPassword = "";
  String formEmail = "";

  // Desligamento do Access Point.
  unsigned long OffAP_previousMillis = 0;
  const long OffAP_interval = 10 * 60 * 1000;  // 5 minutos. Quanto tempo o access point vai ficar ativo antes de ser automaticamente desligado
  bool IsAccessPointActive = false;

  struct AppParam {
    char mac[20] = "";
    char ssid[100] = "";
    char pass[50] = "";
    char email[300] = "";
  };

  void SaveFormData(String wifiName, String wifiPass, String email) {
    lcd->Log("Updating EEPROM");

    String MacAddress = this->Net->GetMacId();
    EEBoom<AppParam> ee;
    ee.begin(1000, 5);

    lcd->Log(MacAddress);
    lcd->Log(email);

    char* m = util.stringToCharArray(MacAddress);
    char* n = util.stringToCharArray(wifiName);
    char* p = util.stringToCharArray(wifiPass);
    char* e = util.stringToCharArray(email);

    strcpy(ee.data.mac, m);   // Mac de Controle
    strcpy(ee.data.ssid, n);  // SSID
    strcpy(ee.data.pass, p);
    strcpy(ee.data.email, e);

    ee.commit();

    util.freeCharArray(m);
    util.freeCharArray(n);
    util.freeCharArray(p);
    util.freeCharArray(e);

    lcd->Log("EEPPROM Updated");
  }

  void LoadFormData() {
    String MacAddress = this->Net->GetMacId();

    EEBoom<AppParam> ee;
    ee.begin(1000, 5);

    if (String(ee.data.mac) == MacAddress) {
      this->formWifiName = String(ee.data.ssid);
      this->formWifiPassword = String(ee.data.pass);
      this->formEmail = String(ee.data.email);
    }
  }

  String htmlHeader(String SubTitle) {
    String CSS = "div,h1{padding:.5em}body,label{color:#333}small{font-size:0.5em}label,nav,nav b{display:block}input,select{width:100%}article{background:#f2f2f2;padding:1.3em}input{font-family:monospace}body{font-family:Century Gothic,sans-serif;font-size:18px;line-height:24px;margin:0;padding:0}h1{margin:.5em 0 0}input, select{padding:9px 10px;margin:8px 0;box-sizing:border-box;border-radius:10px;border:1px solid #555}label{font-style:italic;font-weight:700}nav{background:#06f;color:#fff;font-size:1.3em;padding:1em}nav b{font-size:1.5em;margin-bottom:.5em}";
    String result = "<!DOCTYPE html><head><title>" + this->Title + "</title><meta name=viewport content=\"width=device-width,initial-scale=1\"><style>"
                    + CSS + "</style><meta charset=\"UTF-8\"></head><body><nav><b>"
                    + this->Title + "</b> " + SubTitle + "<br/><small>" + this->Net->GetMacId() + "</small>";

    if (this->UserWiFiIsConnected) {
      result += R"(<br/><strong style="color:gold;font-size=0.4em;">The module is currently connected...</strong>)";
    }

    result += "</nav><div>";

    return result;
  }

  String JSSendForm() {
    String result = R"(
      <script>
        document.getElementById('disconnect').addEventListener('submit', function(event) {
          fetch('/disconnect', { method: 'GET' });
        });

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
    String Body = "Please provide the necessary information to connect the module to your internet and register it to your account on our website.";
    String htmlResult = htmlHeader("Module Configuration") + "<div>" + Body + "</ol></div><div><form id=regform><label>Select your WiFi Network:</label>"
                        + this->htmlSelectWiFi
                        + R"(<label>WiFi Password:</label><input type=text name=wifipass value=")"
                        + this->formWifiPassword
                        + R"("></input>)"
                        + R"(<label>Your eMail:</label><input type=email name=email value=")"
                        + this->formEmail
                        + R"("></input>)"
                        + R"(<input type=submit id=submit value="Verify the connection"></input></form>)"
                        + R"(<br/><strong id=msgresult></strong>)"
                        + R"(<form id=disconnect><input type=submit id=submit value=Close></input></form>)"
                        + htmlFooter();

    return htmlResult;
  }

  bool TryConnectWiFi(bool force = false) {
    if (((!this->UserWiFiIsConnected || force) && !this->formWifiName.isEmpty() && !this->formWifiPassword.isEmpty())) {
      lcd->Log("Con: " + this->formWifiName);
      this->UserWiFiIsConnected = this->Net->ConnectWiFi(this->formWifiName, this->formWifiPassword);
      if (this->UserWiFiIsConnected) {
        lcd->Log("Con: Success");
      } else {
        lcd->Log("Con: Failure");
      }
      return this->UserWiFiIsConnected;
    } else {
      return false;
    }
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
    } else if (!request.containsKey("email") || request["email"].isNull() || request["email"].as<String>().isEmpty()) {
      result["message"] = "Provide your email address to register the module.";
    } else {
      result["wifiSsid"] = request["wifiname"];
      result["wifiPass"] = request["wifipass"];
      result["email"] = request["email"];

      if (!util.isValidEmail(result["email"])) {
        result["message"] = "The email address you entered is invalid. Please check it.";
      } else {
        // Tudo informado, vamos testar o WiFi
        lcd->Log("Con: " + String(result["wifiSsid"]));
        result["success"] = this->Net->ConnectWiFi(result["wifiSsid"], result["wifiPass"]);

        if (result["success"]) {
          lcd->Log("Con: Success");

          this->SaveFormData(result["wifiSsid"], result["wifiPass"], result["email"]);  // Sucesso! Salva os dados na EEProm
          result["message"] = "Data updated successfully. Wi-Fi connected!";
          this->UserWiFiIsConnected = true;
        } else {
          lcd->Log("Con: Failure");
          delay(1000);
          lcd->Log("Con: Previous Network");
          this->TryConnectWiFi(true);

          result["message"] = "Connection to the selected Wi-Fi network failed. Please verify the password.";
        }
      }
    }

    String strResult;
    serializeJson(result, strResult);

    Serial.println(strResult);

    return strResult;
  }

  String disconnect() {
    this->DisableAccessPoint();

    return "";
  }

  void DisableAccessPoint() {
    if (this->IsAccessPointActive) {
      dnsServer.stop();
      webServer.stop();

      this->IsAccessPointActive = false;

      WiFi.disconnect();
      delay(100);

      lcd->Log("Access Point Disabled");
      lcd->Log("Restart to Enable");

      this->TryConnectWiFi(true);
    }
  }

  void StartAccessPoint() {
    if (!this->IsAccessPointActive) {
      this->htmlSelectWiFi = this->Net->GetWifiNetworksAsHtmlSelect(this->formWifiName);  // Redes WiFi disponíveis

      WiFi.mode(WIFI_AP_STA);                                      // Access Point e WiFi ao mesmo tempo!
      WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));  // APIP Inicializado com 172.0.0.1
      WiFi.softAP(currentSSID);                                    // Nome da Rede

      dnsServer.start(DNS_PORT, "*", APIP);  // DNS spoofing (Only for HTTP)

      lcd->Log("Access Point Active");

      webServer.on("/reg", [this]() {
        webServer.send(HTTP_CODE, "application/json", this->jsonRegister());
      });

      webServer.on("/disconnect", [this]() {
        webServer.send(HTTP_CODE, "application/json", this->disconnect());
      });

      webServer.onNotFound([this]() {
        webServer.send(HTTP_CODE, "text/html", this->htmlPageIndex());
      });

      // Finaliza a habilitação do AccessPoint
      webServer.begin();

      this->IsAccessPointActive = true;
    }
  }

public:
  Portal(WiFiNetwork* n, LcdFreeWrite* l, String title) {
    this->Net = n;
    this->lcd = l;
    this->Title = title;
  }

  void Setup(String WiFiName) {
    this->LoadFormData();

    String MacId = this->Net->GetMacId();
    String MacCode = util.MacToString(MacId.c_str());
    this->currentSSID = WiFiName + " " + MacCode;

    lcd->Log("Module ID: " + MacCode);
    lcd->Log("SSID: " + currentSSID);
    lcd->SetTopMessage(this->currentSSID);

    // Tenta se conectar no Rede WiFi atualmente configurada
    TryConnectWiFi();

    if (!this->UserWiFiIsConnected) {
      this->StartAccessPoint();
    }
  }

  void Loop() {
    if (this->IsAccessPointActive) {
      dnsServer.processNextRequest();
      webServer.handleClient();

      // Aguarda alguns segundos e desativa o AccessPoint
      unsigned long currentMillis = millis();
      if (currentMillis - OffAP_previousMillis >= OffAP_interval) {
        OffAP_previousMillis = currentMillis;
        // Chame o método aqui
        this->DisableAccessPoint();
      }
    }
  }
};