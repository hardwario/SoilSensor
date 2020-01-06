/*

Soil Moisture Sensor
====================

Arduino Library for HARDWARIO Soil Sensor
Author: podija https://github.com/podija
        hubmartin https://github.com/hubmartin

HARDWARIO is a digital maker kit developed by https://www.hardwario.com/

Product: https://shop.hardwario.com/soil-sensor/
Specs: https://developers.bigclown.com/hardware/about-soil-moisture-sensor
Firmware: https://developers.bigclown.com/firmware/how-to-soil-moisture-sensor
Forum: https://forum.bigclown.com/

MIT License

This example uses HARDWARIO Soil Sensor for soil moisture and temperature measurement. Measured data are printed on serial port in text format. 

*/
#include <OneWire.h>
#include <SoilSensor.h>

// Add a 4k7 pull-up resistor to this pin
#define SOIL_SENSOR_PIN 7

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
  
  uint16_t moisture;
  soilSensor.readMoistureRaw(&moisture);
  Serial.println(moisture);
   
  soilSensor.sleep();
  delay(50); 
}
