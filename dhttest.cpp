#include <DHT.h>
#include <Adafruit_Sensor.h>
// DHT sensor configuration
#define DHTPIN 22       // Pin where the DHT11 is connected
#define DHTTYPE DHT11  // DHT 11 sensor type

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Initialize the serial monitor
  Serial.begin(115200);
  Serial.println("DHT11 sensor initialization");

  // Start DHT sensor
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements
  delay(2000);

  // Reading temperature and humidity from the DHT11 sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if any reads failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print values to the serial monitor
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
}
