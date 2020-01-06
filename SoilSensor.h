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
#ifndef SoilSensor_h
#define SoilSensor_h

#include "Arduino.h"
#include <OneWire.h>
#include "DS28E17.h"

#define TMP112_ADDRESS    0x48
#define TMP112_ENABLE_SLEEP {0x01, 0x80}
#define TMP112_MEASURE {0x81, 0x80}
#define TMP112_REGISTER 0x01

#define ZSSC3123_ADDRESS  0x28
#define ZSSC3123_MEASURE  0x00

#define EEPROM_ADDRESS    0x51
#define EEPROM_BANK_A     0
#define EEPROM_BANK_B     0x080
#define EEPROM_BANK_C     0x100

#define BC_SOIL_SENSOR_SIGNATURE 0xdeadbeef
#define BC_SOIL_SENSOR_MIN 1700
#define BC_SOIL_SENSOR_MAX 3000
#define BC_SOIL_SENSOR_REV_NO_EEPROM 0x0104
#define BC_SOIL_SENSOR_REV_WITH_EEPROM 0x0104

#define SEARCH_TIMEOUT 50

/**
 * @brief Soil sensor header stored in EEPROM.
 */
typedef struct
{  
    uint32_t signature; //! @brief Signature 0xdeadbeef
    uint8_t version;    //! @brief Data version 
    uint8_t length;     //! @brief Data length
    uint16_t crc;       //! @brief Data CRC

} soilSensorEepromHeader;

/**
 * @brief Soil sensor calibration data stored in EEPROM.
 */
typedef struct
{
    uint8_t product;          //! @brief Product number
    uint16_t revision;        //! @brief Hardware revision
    char label[16 + 1];       //! @brief Label
    uint16_t calibration[11]; //! @brief Calibration points
} soilSensorEeprom;

/**
 * @brief Soil sensor data.
 */
typedef struct
{
    uint8_t address[8] = {0,0,0,0,0,0,0,0}; //! @brief Sensor address
    soilSensorEeprom eeprom;                //! @brief EEPROM calibration data
}soilSensorT;

class SoilSensor
{
  public:
    /**
      * @brief       Constructor of SoilSensor class.
      */
    SoilSensor(OneWire *oneWire);
    
    /**
      * @brief       Search and init sensor.
      * @return      True if searched, otherwise false.
      */
    bool begin();
    
    /**
     * @brief       Wake up asleep soil sensor.
     */
    void wakeUp();
    
    /**
     * @brief       Put soil sensor into sleep mode.
     */
    void sleep();

    /**
      * @brief       Read raw moisture from soil sensor.
      * @param[out]  raw   raw moisture to be read
      * @return      True if the read was successful, otherwise false.
      */
    bool readMoistureRaw(uint16_t *moisture);

    /**
      * @brief       Experimental: Read percentage moisture from soil sensor.
      * @param[out]  moisture   moisture to be read
      * @return      True if the read was successful, otherwise false.
      */
    bool readMoisture(uint8_t *moisture);
    
    /**
     * @brief       Read temperature in Celsius from soil sensor.
     * @param[out]  temperature   temperature to be read
     * @return      True if the read was successful, otherwise false.
     */
    bool readTemperatureCelsius(float *temperature); 
    
    /**
     * @brief       Read temperature in Kelvin from soil sensor.
     * @param[out]  temperature   temperature to be read
     * @return      True if the read was successful, otherwise false.
     */
    bool readTemperatureKelvin(float *temperature); 
    
    /**
     * @brief       Read temperature in Fahrenheit from soil sensor.
     * @param[out]  temperature   temperature to be read
     * @return      True if the read was successful, otherwise false.
     */
    bool readTemperatureFahrenheit(float *temperature);
    
  private:
    /**
     * @brief       Pointer to OneWire object.
     */
    OneWire *oneWire;
    
    /**
     * @brief       DS28E17 (1-wire <-> I2C converter) object.
     */
    DS28E17 ds28e17;
    
    /**
     * @brief       Instance of soilSensorT structure.
     */
    soilSensorT sensor;
    
    /**
     * @brief       Read values from EEPROM memmory on sensor.
     * @param       address   address from which is readed
     * @param[out]  buffer    buffer for readed data
     * @param       length    required data length
     * @return      True if the read was successful, otherwise false.
     */
    bool _EEPROMRead(uint8_t address, void *buffer, size_t length);
    
    /**
     * @brief       Fill regularly distributed calibration.
     */ 
    void _EEPROMFill();
    
    /**
     * @brief       Load calibartion values from EEPROM memmory on sensor.
     * @return      True if the calibration is loaded, false if there is error and regularly distributed calibration is filled.
     */  
    bool _EEPROMLoad();
    
    /**
     * @brief       Read raw capacity from ZSSC3123 circuit.
     * @param[out]  cap    capacity to be read
     * @return      True if the read was successful, otherwise false.
     */ 
    bool _ZSSC3123ReadRaw(uint16_t *cap);
    
    /**
     * @brief       Enable sutdown (power save) mode of TMP112.
     * @return      True if enable was successful, otherwise false.
     */ 
    bool _TMP112EnableShutdownMode();
    
    /**
     * @brief       Start measure (wakeup from power save mode) of TMP112.
     * @return      True if request was successful, otherwise false.
     */ 
    bool _TMP112StartOneShotConversion();
    
    /**
     * @brief       Read moisture from soil sensor tranformed to defined interval.
     * @param[out]  moisture  moisture to be read
     * @param       min       minimal value of defined interval
     * @param       max       maximal value of defined interval
     * @return      True if the read was successful, otherwise false.
     */
    bool readMoistureInterval(uint16_t *moisture, uint16_t min, uint16_t max);
     
};

#endif