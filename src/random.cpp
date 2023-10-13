#include "random.h"

int random_init(SensoriandoSensorDatum **datum, char *uuid)
{
    byte res;

    pinMode(GPIO_DATA, OUTPUT);
    LOGGER_RAN("Lenght: %i", RANDOM_LEN * sizeof(*datum));
    LOGGER_RAN("Random init...");

    *datum = (SensoriandoSensorDatum *)malloc(RANDOM_LEN * sizeof(SensoriandoSensorDatum));
    res = *datum != NULL;

    if ( res ) {
        LOGGER_RAN("Memory allocated...");
        LOGGER_RAN("Size struct: %i", sizeof(SensoriandoSensorDatum));
        LOGGER_RAN("Size alloc: %i", sizeof(*datum));
        LOGGER_RAN("Size array: %i", sizeof(*datum) / sizeof(SensoriandoSensorDatum));

        for (int i = 0; i < RANDOM_LEN; i++) {
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

int random_read(SensoriandoSensorDatum **datum, long *elapsed) 
{
    float value;  
    int res=0;

    if ( (millis() - (*elapsed)) > UPDATEELAPSED ) {
        digitalWrite(GPIO_DATA, 1);
        *elapsed = millis();
 
        LOGGER_RAN("Random read...");
        value = random(1, 100) / PI;

        if ( value ) {
            LOGGER_RAN("Value %f", value);

            for (int i=0; i<RANDOM_LEN; i++) {
                switch ( i ) {
                    case 0:     (*datum + i)->id = RANDOM_ID;
                                (*datum + i)->value = value;
                                res++;
                                break;
                    default:    res=0;
                                break;
                }
            }        
        } else {
            LOGGER_RAN("Value ERROR");
        }
    }

    digitalWrite(GPIO_DATA, 0);
    return res;
}

