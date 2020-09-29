#include "weather.h"

int weather_init(SensoriandoSensorDatum *datum)
{
#ifdef WEATHER_DEBUG
Serial.println("weather init...");
#endif

    datum = (SensoriandoSensorDatum *)malloc(WEATHER_LEN * sizeof(SensoriandoSensorDatum));

#ifdef WEATHER_DEBUG
Serial.println("Memory allocated...");
Serial.print("Size struct: ");Serial.println(sizeof(SensoriandoSensorDatum));
Serial.print("Size pointer: ");Serial.println(sizeof(datum));
Serial.print("Size array: ");Serial.println(sizeof(datum)/sizeof(SensoriandoSensorDatum));
#endif

    return datum != NULL;
}

int weather_read(SensoriandoSensorDatum *datum) 
{
#ifdef WEATHER_DEBUG
Serial.println("weather read...");
#endif
    SimpleDHT11 dht(GPIO_WEATHER);
    byte temperature, humidity;  
    int res=0;
    int dht11;
    int i;

    dht11 = dht.read(&temperature, &humidity, NULL);

#ifdef WEATHER_DEBUG
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

    if (dht11 == SimpleDHTErrSuccess) {
#ifdef WEATHER_DEBUG
Serial.print(temperature); Serial.print(" *C, ");Serial.print(humidity); Serial.println(" %");
#endif

        for (i=0; i<WEATHER_LEN; i++) {
            datum[i].stx = STX;
            datum[i].dt = NULL;
            datum[i].etx = ETX;

            switch ( i ) {
                case 0: datum[i].id = TEMPERATURE_ID;
                        datum[i].value = temperature;
                        res++;
                        break;
                case 1: datum[i].id = HUMIDITY_ID;
                        datum[i].value = humidity;
                        res++;
                        break;
                default: res=0;
                         break;
            }
        }
    }

    delay(1000); //need, less than result error
    return res;
}

