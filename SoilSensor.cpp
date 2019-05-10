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

//konstruktor, vyzaduje OneWire a DS28E17
SoilSensor::SoilSensor(OneWire *oneWireW)
{
  oneWire = oneWireW;
  ds28e17 = DS28E17(oneWireW);
}

//vyhledani a inicializace senzoru
bool SoilSensor::begin()
{
  oneWire->reset();
  oneWire->reset();

  oneWire->reset_search();
  oneWire->target_search(0x19);
  int timeout = 0;
  while (bc_soil_sensor.address[0]==0){//z nejakeho duvodu tady dochazi k tomu, ze se adresa nenajde napoprve
    oneWire->reset_search();
    oneWire->search(bc_soil_sensor.address);
    /*Serial.print(" Address:  ");
    for (int i=0;i<8;i++){
      Serial.print(bc_soil_sensor.address[i], HEX);
      Serial.print(" ");
    }
    Serial.println();*/
    timeout ++;
    if (timeout == SEARCH_TIMEOUT)
      return false;
  }
  
  ds28e17.setAddress(bc_soil_sensor.address);
  
  _EEPROMLoad();
  
  _TMP112EnableShutdownMode();  
  
  return true;
}

//cteni vlhkosti v procentech, prepocteno podle kalibrace
bool SoilSensor::readMoisture(uint8_t *moisture)
{
  uint16_t raw;
  if (!_ZSSC3123ReadRaw(&raw)){
    return false;
  }
  
  uint16_t *calibration = bc_soil_sensor.eeprom.calibration;
  
  //Serial.println(raw);

  for (int i = 0; i < 11; i++){
    if (raw < calibration[i]){
      if (i == 0){
        *moisture = 0;
        return true;
      }

      *moisture = (((raw - calibration[i - 1]) * 10) / (calibration[i] - calibration[i - 1])) + (10 * (i - 1));
      return true;
    }
  }

  *moisture = 100;

  return true;    
}

//funkce pro probuzeni uspaneho prevodniku
void SoilSensor::wakeUp()
{
  ds28e17.wakeUp();   
}

//uspani prevodniku, probouzi se nabeznou hranou
void SoilSensor::sleep()
{  
  ds28e17.enableSleepMode();  
}

//cteni teploty Celsius
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

//cteni teploty kelvin
bool SoilSensor::readTemperatureKelvin(float *temperature)
{
  float temp;
  if (!readTemperatureCelsius(&temp) ) {
    return false;
  }
  
  *temperature = temp * 1.8 + 32;  
  return true;
}
 
//cteni teploty Fahrenheit
bool SoilSensor::readTemperatureFahrenheit(float *temperature)
{
  float temp;
  if (!readTemperatureCelsius(&temp) ) {
    return false;
  }
  
  *temperature = temp + 273.15; 
  return true;  
}

//cteni kalibracnich hodnot z EEPROM    
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
  uint8_t *p = buffer;

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

//pokud se nepovede precist z EEPROM, vyplni se kalibrace rovnomerne
void SoilSensor::_EEPROMFill()
{
    bc_soil_sensor.eeprom.product = 0;
    bc_soil_sensor.eeprom.revision = BC_SOIL_SENSOR_REV_NO_EEPROM;

    bc_soil_sensor.eeprom.calibration[0] = BC_SOIL_SENSOR_MIN;
    bc_soil_sensor.eeprom.calibration[10] = BC_SOIL_SENSOR_MAX;

    uint16_t step = (bc_soil_sensor.eeprom.calibration[10] - bc_soil_sensor.eeprom.calibration[0]) / 11;

    for (int i = 1; i < 10; i++)
    {
        bc_soil_sensor.eeprom.calibration[i] = bc_soil_sensor.eeprom.calibration[i - 1] + step;
    }

    memset(bc_soil_sensor.eeprom.label, 0, sizeof(bc_soil_sensor.eeprom.label));    
}

//nacteni eeprom vcetne kontroly, zda to proslo a pripadne nahrani nahradni kalibrace
bool SoilSensor::_EEPROMLoad()
{
  bool error = false;

  bc_soil_sensor_eeprom_header_t header;

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

  if (header.length != sizeof(bc_soil_sensor_eeprom_t)){
    error = true;
  }

  if (!_EEPROMRead(sizeof(header), &bc_soil_sensor.eeprom, sizeof(bc_soil_sensor_eeprom_t))){
    error = true;
  }

  if (header.crc != oneWire->crc16(&bc_soil_sensor.eeprom.product, sizeof(bc_soil_sensor_eeprom_t), 0)){
    error = true;
  }

  if (error){
    _EEPROMFill();
  }
  /*
  Serial.print("EEPROM data: ");
  Serial.print(bc_soil_sensor.eeprom.product, HEX);
  Serial.print(" ");
  Serial.print(bc_soil_sensor.eeprom.revision, HEX);
  Serial.print(" ");
  Serial.print(bc_soil_sensor.eeprom.label);
  Serial.print(" ");
  for (int z=0;z<11;z++)
  {
    Serial.print(bc_soil_sensor.eeprom.calibration[z]);
    Serial.print(" ");
  }
  Serial.println();
  */
  
  return error;    
}

//cteni raw kapacity z ZSSC32123
bool SoilSensor::_ZSSC3123ReadRaw(uint16_t *cap)
{
  uint8_t data[1] = {0x00};
  ds28e17.write(ZSSC3123_ADDRESS, data, 1);//measure request

  uint8_t readedBuff[2];
  if (ds28e17.read(ZSSC3123_ADDRESS, readedBuff, 2) == false){
    return false;
  }

  uint16_t readed = readedBuff[0] << 8 | readedBuff[1];
  
  if ((readed & 0xc000) == 0)
  {
    *cap = readed & 0xBFFF; //read and convert data
    return true;
  }
  return false;    
}

//umozni, aby se TMP112 po kazdem precteni uspalo dokud neprijde pozadavek
bool SoilSensor::_TMP112EnableShutdownMode()
{
  uint8_t data[2] = {0x01, 0x80};
  return ds28e17.memoryWrite(TMP112_ADDRESS, 0x01, data, 2);  
}

//pozadavek na cteni
bool SoilSensor::_TMP112StartOneShotConversion()
{
  uint8_t data[2] = {0x81, 0x80};
  return ds28e17.memoryWrite(TMP112_ADDRESS, 0x01, data, 2);  
}
