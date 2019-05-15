/*

Soil Moisture Sensor
====================

Arduino Library for BigClown Soil Sensor
Author: podija https://github.com/podija

BigClown is a digital maker kit https://www.bigclown.com/ developed by https://www.hardwario.com/

Product: https://shop.bigclown.com/soil-moisture-sensor/
Specs: https://developers.bigclown.com/hardware/about-soil-moisture-sensor
Firmware: https://developers.bigclown.com/firmware/how-to-soil-moisture-sensor
Forum: https://forum.bigclown.com/

MIT License

This example uses BigClown Soil Sensor for soil moisture and temperature measurement. Measured data are printed on serial port in text format. 

*/
#include <OneWire.h>
#include <SoilSensor.h>
#include <DS28E17.h>

// Add a 4k7 pull-up resistor to this pin
#define SOIL_SENSOR_PIN 8

OneWire oneWire(SOIL_SENSOR_PIN);
SoilSensor soilSensor(&oneWire);

void setup() 
{
  Serial.begin(9600);
  Serial.println("BigClown Soil Sensor Example");
  
  soilSensor.begin();
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
   
  soilSensor.sleep();
  delay(2000); 
}
