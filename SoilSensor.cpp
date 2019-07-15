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

*/

#include "Arduino.h"
#include "SoilSensor.h"


SoilSensor::SoilSensor(OneWire *oneWireW)
{
  oneWire = oneWireW;
  ds28e17 = DS28E17(oneWireW);
}


bool SoilSensor::begin()
{
  oneWire->reset();
  oneWire->reset();

  oneWire->reset_search();
  oneWire->target_search(0x19);
  int timeout = 0;
  while (sensor.address[0]==0){//sensor is not finded on first attempt
    oneWire->reset_search();
    oneWire->search(sensor.address);
    /*Serial.print(" Address:  ");
    for (int i=0;i<8;i++){
      Serial.print(sensor.address[i], HEX);
      Serial.print(" ");
    }
    Serial.println();*/
    timeout ++;
    if (timeout == SEARCH_TIMEOUT)
      return false;
  }
  
  ds28e17.setAddress(sensor.address);
  
  _EEPROMLoad();
  
  _TMP112EnableShutdownMode();  
  
  return true;
}


bool SoilSensor::readMoisture(uint8_t *moisture)
{
  uint16_t moist;
  
  if (!readMoistureInterval(&moist, 0, 100)){
    return false;
  }
  
  *moisture = moist;


  return true;    
}


bool SoilSensor::readMoistureInterval(uint16_t *moisture, uint16_t min, uint16_t max)
{
  uint16_t raw;
  if (!_ZSSC3123ReadRaw(&raw)){
    return false;
  }
  
  uint16_t *calibration = sensor.eeprom.calibration;
  
  //Serial.println(raw);
  
  int step = (max - min)/10;

  for (int i = 0; i < 11; i++){
    if (raw < calibration[i]){
      if (i == 0){
        *moisture = min;
        return true;
      }

      *moisture = (((raw - calibration[i - 1]) * step) / (calibration[i] - calibration[i - 1])) + (step * (i - 1));
      return true;
    }
  }

  *moisture = max;

  return true;    
}


void SoilSensor::wakeUp()
{
  ds28e17.wakeUp();   
}


void SoilSensor::sleep()
{  
  ds28e17.enableSleepMode();  
}


bool SoilSensor::readTemperatureCelsius(float *temperature) 
{
  _TMP112StartOneShotConversion();
  delay(1);
  
  uint8_t readed[2];
  if (!ds28e17.memoryRead(TMP112_ADDRESS, 0x00, readed, 2)){
    return false;
  }
   
  uint16_t temperatureRaw = readed[0] << 8 | readed[1];
  
  *temperature = (temperatureRaw >> 4) * 0.0625;
  
  return true;
}


bool SoilSensor::readTemperatureFahrenheit(float *temperature)
{
  float temp;
  if (!readTemperatureCelsius(&temp) ) {
    return false;
  }
  
  *temperature = temp * 1.8 + 32;  
  return true;
}

 
bool SoilSensor::readTemperatureKelvin(float *temperature)
{
  float temp;
  if (!readTemperatureCelsius(&temp) ) {
    return false;
  }
  
  *temperature = temp + 273.15; 
  return true;  
}

   
bool SoilSensor::_EEPROMRead(uint8_t address, void *buffer, size_t length)
{
  uint8_t a[8];
  uint8_t b[sizeof(a)];
  uint8_t c[sizeof(a)];

  if ((EEPROM_BANK_A + address + length) >= EEPROM_BANK_B)
  {
    return false;
  }

  size_t j = 0;
  uint8_t *p = (uint8_t *)buffer;

  for (size_t i = 0; i < length; i += sizeof(a))
  {
    size_t len = length - i > sizeof(a) ? sizeof(a) : length - i;

    uint32_t memoryAddress = address + i + EEPROM_BANK_A;

    if (!ds28e17.memoryRead(EEPROM_ADDRESS, memoryAddress, a, len))
    {
      return false;
    }        

    memoryAddress = address + i + EEPROM_BANK_B;

    if (!ds28e17.memoryRead(EEPROM_ADDRESS, memoryAddress, b, len))
    {
      return false;
    }

    memoryAddress = address + i + EEPROM_BANK_C;

    if (!ds28e17.memoryRead(EEPROM_ADDRESS, memoryAddress, c, len))
    {
      return false;
    }

    for (j = 0; j < len; j++)
    {
      *p++ = (a[j] & b[j]) | (a[j] & c[j]) | (b[j] & c[j]);
    }
  }

  return true;    
}

 
void SoilSensor::_EEPROMFill()
{
    sensor.eeprom.product = 0;
    sensor.eeprom.revision = BC_SOIL_SENSOR_REV_NO_EEPROM;

    sensor.eeprom.calibration[0] = BC_SOIL_SENSOR_MIN;
    sensor.eeprom.calibration[10] = BC_SOIL_SENSOR_MAX;

    uint16_t step = (sensor.eeprom.calibration[10] - sensor.eeprom.calibration[0]) / 11;

    for (int i = 1; i < 10; i++)
    {
        sensor.eeprom.calibration[i] = sensor.eeprom.calibration[i - 1] + step;
    }

    memset(sensor.eeprom.label, 0, sizeof(sensor.eeprom.label));    
}


bool SoilSensor::_EEPROMLoad()
{
  bool error = false;

  soilSensorEepromHeader header;

  if (!_EEPROMRead(0, &header, sizeof(header))){
    error = true;
  }
  /*
  Serial.print("EEPROM header: ");
  Serial.print(header.signature, HEX);
  Serial.print(" ");
  Serial.print(header.version, HEX);
  Serial.print(" ");
  Serial.print(header.length);
  Serial.print(" ");
  Serial.print(header.crc);
  Serial.println();
  */
  if (header.signature != BC_SOIL_SENSOR_SIGNATURE){
    error = true;
  }

  if (header.version != 1){
    error = true;
  }

  if (header.length != sizeof(soilSensorEeprom)){
    error = true;
  }

  if (!_EEPROMRead(sizeof(header), &sensor.eeprom, sizeof(soilSensorEeprom))){
    error = true;
  }

  if (header.crc != oneWire->crc16(&sensor.eeprom.product, sizeof(soilSensorEeprom), 0)){
    error = true;
  }

  if (error){
    _EEPROMFill();
  }
  /*
  Serial.print("EEPROM data: ");
  Serial.print(sensor.eeprom.product, HEX);
  Serial.print(" ");
  Serial.print(sensor.eeprom.revision, HEX);
  Serial.print(" ");
  Serial.print(sensor.eeprom.label);
  Serial.print(" ");
  for (int z=0;z<11;z++)
  {
    Serial.print(sensor.eeprom.calibration[z]);
    Serial.print(" ");
  }
  Serial.println();
  */
  
  return error;    
}


bool SoilSensor::_ZSSC3123ReadRaw(uint16_t *cap)
{
  uint8_t data[1] = {ZSSC3123_MEASURE};
  ds28e17.write(ZSSC3123_ADDRESS, data, 1);

  uint8_t readedBuff[2];
  if (ds28e17.read(ZSSC3123_ADDRESS, readedBuff, 2) == false){
    return false;
  }

  uint16_t readed = readedBuff[0] << 8 | readedBuff[1];
  
  if ((readed & 0xc000) == 0)
  {
    *cap = readed & 0xBFFF; 
    return true;
  }
  return false;    
}


bool SoilSensor::_TMP112EnableShutdownMode()
{
  uint8_t data[2] = TMP112_ENABLE_SLEEP;
  return ds28e17.memoryWrite(TMP112_ADDRESS, TMP112_REGISTER, data, 2);  
}


bool SoilSensor::_TMP112StartOneShotConversion()
{
  uint8_t data[2] = TMP112_MEASURE;
  return ds28e17.memoryWrite(TMP112_ADDRESS, TMP112_REGISTER, data, 2);  
}
