#include <OneWire.h>
#include <BC_Soil_Sensor.h>
#include <DS28E17.h>

#define SOIL_SENSOR_PIN 7

OneWire oneWire(SOIL_SENSOR_PIN);
DS28E17 ds28e17(&oneWire);
BC_Soil_Sensor soilSensor(&oneWire, &ds28e17);

void setup() 
{
  Serial.begin(9600);
  Serial.println("BigClown Soil Sensor Example");
  
  soilSensor.begin();

  delay(300);  
}

void loop()
{
  soilSensor.wakeUp();
  
  float temperature;
  soilSensor.readTemperatureCelsius(&temperature);
  Serial.print("Temperature:  ");
  Serial.print(temperature);
  Serial.println("°C");

  soilSensor.readTemperatureFahrenheit(&temperature);
  Serial.print("Temperature:  ");
  Serial.print(temperature);
  Serial.println("°F");

  soilSensor.readTemperatureKelvin(&temperature);
  Serial.print("Temperature:  ");
  Serial.print(temperature);
  Serial.println("K");
  
  uint8_t moisture;
  soilSensor.readMoisture(&moisture);
  Serial.print("Moisture:  ");
  Serial.print(moisture);
  Serial.println("%");
  Serial.println();
   
  soilSensor.enableSleepMode();
  delay(10000); 
}
