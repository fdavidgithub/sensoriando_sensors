#include "weather.h"

int weather_init(SensoriandoSensorDatum **datum)
{
#ifdef DEBUG_WEATHER
Serial.print(WEATHER_LEN * sizeof(*datum));
Serial.println("[DEBUG WEATHER] Weather init...");
#endif

    *datum = (SensoriandoSensorDatum *)malloc(WEATHER_LEN * sizeof(SensoriandoSensorDatum));

#ifdef DEBUG_WEATHER
Serial.println("[DEBUG WEATHER] Memory allocated...");
Serial.print("Size struct: ");Serial.println(sizeof(SensoriandoSensorDatum));
Serial.print("Size alloc: ");Serial.println(sizeof(*datum));
Serial.print("Size array: ");Serial.println(sizeof(*datum) / sizeof(SensoriandoSensorDatum));
#endif

    return *datum != NULL;
}

int weather_read(SensoriandoSensorDatum **datum) 
{
    byte temperature, humidity;  
    int res=0;
    int dht11;
    int i;


#ifdef DEBUG_WEATHER
Serial.println("[DEBUG WEATHER] Weather read...");
#endif

    SimpleDHT11 dht(GPIO_WEATHER);
    dht11 = dht.read(&temperature, &humidity, NULL);

    if (dht11 == SimpleDHTErrSuccess) {
#ifdef DEBUG_WEATHER
Serial.print("[DEBUG WEATHER] ");
Serial.print(temperature);Serial.print(" *C, ");
Serial.print(humidity); Serial.println(" %");
#endif

        for (i=0; i<WEATHER_LEN; i++) {
            (*datum + i)->stx = STX;
            (*datum + i)->dt = NULL;
            (*datum + i)->etx = ETX;
            strcpy((*datum + i)->uuid, WEATHER_UUID);

            switch ( i ) {
                case 0: (*datum + i)->id = TEMPERATURE_ID;
                        (*datum + i)->value = KELVIN(temperature);
                        res++;
                        break;
                case 1: (*datum + i)->id = HUMIDITY_ID;
                        (*datum + i)->value = humidity;
                        res++;
                        break;
                default: res=0;
                         break;
            }
        }        
    } else {
#ifdef DEBUG_WEATHER
Serial.print("[DEBUG WEATHER] ");

switch (dht11) {
  case SimpleDHTErrStartLow: Serial.println("Error to wait for start low signal.");break;
  case SimpleDHTErrStartHigh: Serial.println("Error to wait for start high signal.");break;
  case SimpleDHTErrDataLow: Serial.println("Error to wait for data start low signal.");break;
  case SimpleDHTErrDataRead: Serial.println("Error to wait for data read signal.");break;
  case SimpleDHTErrDataEOF: Serial.println("Error to wait for data EOF signal.");break;
  case SimpleDHTErrDataChecksum: Serial.println("Error to validate the checksum.");break;
  case SimpleDHTErrZeroSamples: Serial.println("Error when temperature and humidity are zero, it shouldn't happen.");break;
  case SimpleDHTErrNoPin: Serial.println(" Error when pin is not initialized.");break;    
  default: Serial.print("Error code: ");Serial.println(dht11);break;
}
#endif


    }

    return res;
}

