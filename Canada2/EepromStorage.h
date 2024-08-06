struct EEpromData {
  char devMac[20];
  char ssid[64];
  char password[64];
};

struct WiFiConfig {
  String devMac;
  String ssid;
  String password;
};

class EepromStorage {
private:
  bool loaded = false;
  String MacAddress = WiFi.macAddress();
  EEpromData CurrentData;
public:
  EepromStorage() {
    this->loaded = this->LoadData();
  }
  bool isLoaded() {
    return this->loaded;
  }
  WiFiConfig Data() {
    WiFiConfig result;

    result.devMac = String(this->CurrentData.devMac);
    result.ssid = String(this->CurrentData.ssid);
    result.password = String(this->CurrentData.password);

    return result;
  }
  bool LoadData() {
    EEBoom<EEpromData> result;
    result.begin(1000, 5);

    if (String(result.data.devMac) == this->MacAddress) {
      this->CurrentData = result.data;

      return true;
    } else {
      return false;
    }
  }
  void Reset() {
    Util util;
    EEBoom<EEpromData> ee;
    ee.begin(1000, 5);

    char* m = util.stringToCharArray("xx:xx:xx:xx:xx:xx");
    char* n = util.stringToCharArray("xxxxxxxxxxxxxxxxx");
    char* p = util.stringToCharArray("xxxxxxxxxxxxxxxxx");

    strcpy(ee.data.devMac, m);
    strcpy(ee.data.ssid, n);
    strcpy(ee.data.password, p);

    ee.commit();

    util.freeCharArray(m);
    util.freeCharArray(n);
    util.freeCharArray(p);
  }
  void SaveData(String ssid, String password) {
    Util util;
    EEBoom<EEpromData> ee;
    ee.begin(1000, 5);

    char* m = util.stringToCharArray(this->MacAddress);
    char* n = util.stringToCharArray(ssid);
    char* p = util.stringToCharArray(password);

    strcpy(ee.data.devMac, m);
    strcpy(ee.data.ssid, n);
    strcpy(ee.data.password, p);

    ee.commit();

    util.freeCharArray(m);
    util.freeCharArray(n);
    util.freeCharArray(p);
  }
};
