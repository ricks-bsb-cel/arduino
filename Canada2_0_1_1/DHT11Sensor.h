// Leitora de temperatura e umidade do DHT 11
#include <DHT.h>

struct DHT11SensorValue {
  bool success = false;
  float temperature;
  float humidity;
};

class DHT11Sensor {
private:
  DHT11SensorValue Values;
public:
  DHT11Sensor(byte p) {
    byte SensorPin = p;
  }
  Setup(){
    DHT dht_sensor(this->SensorPin, DHT11);
    dht_sensor.begin();
    dht_sensor.stop();
  }
  bool StartMeasure() {

    float humi = dht_sensor.readHumidity();
    if (!isnan(humi)) {
      Serial.println(humi);
    }

    float temperature_C = dht_sensor.readTemperature();
    if (!isnan(temperature_C)) {
      Serial.println(temperature_C);
    }


    /*
    DHT11SensorValue result;

    if (dht_sensor.measure(&t, &h)) {
      this->temperature = round(t * 10) / 10;
      this->humidity = round(h * 10) / 10;

      Serial.println(this->temperature);
      Serial.println(this->humidity);

      return (true);
    } else {
      return (false);
    }
    */

    return false;
  }
};
