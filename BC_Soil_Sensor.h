#ifndef BC_Soil_Sensor_h
#define BC_Soil_Sensor_h

#include "Arduino.h"
#include <OneWire.h>
#include "DS28E17.h"

#define TMP112_ADDRESS    0x48
#define ZSSC3123_ADDRESS  0x28
#define EEPROM_ADDRESS    0x51
#define EEPROM_BANK_A     0
#define EEPROM_BANK_B     170
#define EEPROM_BANK_C     340

#define BC_SOIL_SENSOR_SIGNATURE 0xdeadbeef
#define BC_SOIL_SENSOR_MIN 1700
#define BC_SOIL_SENSOR_MAX 3000
#define BC_SOIL_SENSOR_REV_NO_EEPROM 0x0104
#define BC_SOIL_SENSOR_REV_WITH_EEPROM 0x0104

#define SEARCH_TIMEOUT 50

typedef struct
{  
    uint32_t signature; //! @brief Signature 0xdeadbeef
    uint8_t version; //! @brief Data Version 
    uint8_t length; //! @brief Data Length
    uint16_t crc; //! @brief Data CRC

} bc_soil_sensor_eeprom_header_t;

typedef struct
{
    uint8_t product; //! @brief Product number
    uint16_t revision; //! @brief Hardware Revision
    char label[16 + 1]; //! @brief Label
    uint16_t calibration[11]; //! @brief Calibration points
} bc_soil_sensor_eeprom_t;


typedef struct
{
    uint8_t address[8] = {0,0,0,0,0,0,0,0};
    bc_soil_sensor_eeprom_t eeprom;
}bc_soil_sensor_t;

class BC_Soil_Sensor
{
  public:
    BC_Soil_Sensor(OneWire *oneWire, DS28E17 *ds28e17W);
    bool begin();
    
    void wakeUp();
    void enableSleepMode();
    
    bool readMoisture(uint8_t *moisture);
 
    bool readTemperatureCelsius(float *temperature); 
    bool readTemperatureKelvin(float *temperature); 
    bool readTemperatureFahrenheit(float *temperature);
    
  private:
    OneWire *oneWire;
    DS28E17 *ds28e17;
    bc_soil_sensor_t bc_soil_sensor;
    bool _EEPROMRead(uint8_t address, void *buffer, size_t length);
    void _EEPROMFill();
    bool _EEPROMLoad();
    bool _ZSSC3123ReadRaw(uint16_t *cap);
    
    bool _TMP112EnableShutdownMode();
    bool _TMP112StartOneShotConversion();
    
};

#endif