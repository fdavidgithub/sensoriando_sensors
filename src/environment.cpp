#include "environment.h"

int environment_init(SensoriandoSensorDatum **datum, char *uuid)
{
    byte res;

    LOGGER_ENV("Length %i", ENVIRONMENT_LEN * sizeof(*datum));
    LOGGER_ENV("Environment init...");

    *datum = (SensoriandoSensorDatum *)malloc(ENVIRONMENT_LEN * sizeof(SensoriandoSensorDatum));
    res = *datum != NULL;

    if ( res ) {
        LOGGER_ENV("Memory allocated...");
        LOGGER_ENV("Size struct: %i", sizeof(SensoriandoSensorDatum));
        LOGGER_ENV("Size alloc: %i", sizeof(*datum));
        LOGGER_ENV("Size array: %i", sizeof(*datum) / sizeof(SensoriandoSensorDatum));

        for (int i = 0; i < ENVIRONMENT_LEN; i++) {
            (*datum + i)->stx = STX;
            (*datum + i)->dt = NULL;
            (*datum + i)->value = NULL;
            (*datum + i)->id = NULL;
            (*datum + i)->etx = ETX;
            strcpy((*datum + i)->uuid, uuid);
        }
    }

    return res;
}

int environment_read(SensoriandoSensorDatum **datum, long *elapsed) 
{
    byte temperature, humidity;  
    int res=0;
    int dht11;

    if ( (millis() - (*elapsed)) > UPDATEELAPSED ) {
        *elapsed = millis();
 
        LOGGER_ENV("Environment read...");

        SimpleDHT11 dht(GPIO_DATA);
        dht11 = dht.read(&temperature, &humidity, NULL);

        if (dht11 == SimpleDHTErrSuccess) {
            LOGGER_ENV("Temp: %x, Humid: %x", temperature, humidity);

            for ( int i=0; i < ENVIRONMENT_LEN; i++ ) {
                switch ( i ) {
                    case 0:     (*datum + i)->id = TEMPERATURE_ID;
                                (*datum + i)->value = KELVIN(temperature);
                                res++;
                                break;
                    case 1:     (*datum + i)->id = HUMIDITY_ID;
                                (*datum + i)->value = humidity;
                                res++;
                                break;
                    default:    res=0;
                                break;
                }
            }        
        } else {
#ifdef DEBUG_ENVIRONMENT
switch (dht11) {
  case SimpleDHTErrStartLow: LOGGER_ENV("Error to wait for start low signal.");break;
  case SimpleDHTErrStartHigh: LOGGER_ENV("Error to wait for start high signal.");break;
  case SimpleDHTErrDataLow: LOGGER_ENV("Error to wait for data start low signal.");break;
  case SimpleDHTErrDataRead: LOGGER_ENV("Error to wait for data read signal.");break;
  case SimpleDHTErrDataEOF: LOGGER_ENV("Error to wait for data EOF signal.");break;
  case SimpleDHTErrDataChecksum: LOGGER_ENV("Error to validate the checksum.");break;
  case SimpleDHTErrZeroSamples: LOGGER_ENV("Error when temperature and humidity are zero, it shouldn't happen.");break;
  case SimpleDHTErrNoPin: LOGGER_ENV(" Error when pin is not initialized.");break;    
  default: LOGGER_ENV("Error code: %i", dht11);break;
}
#endif
        }
   }

    return res;
}

