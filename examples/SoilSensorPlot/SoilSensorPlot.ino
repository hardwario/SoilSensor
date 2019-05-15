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

This example uses BigClown Soil Sensor for soil moisture and temperature measurement. Measured data are printed on serial port in structured format which can be simply vizualized in Arduino Serial Plotter. Just run Serial Plotter and select 9600 baud speed. 

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
  
  soilSensor.begin();
}

void loop()
{
  soilSensor.wakeUp();
  
  float temperature;
  soilSensor.readTemperatureCelsius(&temperature);
  Serial.print(temperature);
  
  Serial.print(" ");
  
  uint8_t moisture;
  soilSensor.readMoisture(&moisture);
  Serial.println(moisture);
   
  soilSensor.sleep();
  delay(2000); 
}
