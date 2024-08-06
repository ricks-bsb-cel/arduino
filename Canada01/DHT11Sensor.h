// Leitora de temperatura e umidade do DHT 11
#include <DHT_Async.h>

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const byte DHT_SENSOR_PIN = D1;
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

      Serial.println(this->temperature);
      Serial.println(this->humidity);

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
    return String(this->temperature, 1) + "C";
  }
  String getHumidityAsString() const {
    return String(this->humidity, 1) + "%";
  }
};
