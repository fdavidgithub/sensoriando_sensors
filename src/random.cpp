#include "random.h"

int random_init(SensoriandoSensorDatum **datum, char *uuid)
{
    byte res;

#ifdef DEBUG_RANDOM
Serial.print(RANDOM_LEN * sizeof(*datum));
Serial.println("[DEBUG RANDOM] Random init...");
#endif

    *datum = (SensoriandoSensorDatum *)malloc(RANDOM_LEN * sizeof(SensoriandoSensorDatum));
    res = *datum != NULL;

    if ( res ) {
#ifdef DEBUG_RANDOM
Serial.println("[DEBUG RANDOM] Memory allocated...");
Serial.print("Size struct: ");Serial.println(sizeof(SensoriandoSensorDatum));
Serial.print("Size alloc: ");Serial.println(sizeof(*datum));
Serial.print("Size array: ");Serial.println(sizeof(*datum) / sizeof(SensoriandoSensorDatum));
#endif

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
        *elapsed = millis();
 
#ifdef DEBUG_RANDOM
Serial.println("[DEBUG RANDOM] Random read...");
#endif

        value = random(1, 100) / PI;

        if ( value ) {
#ifdef DEBUG_RANDOM
Serial.print("[DEBUG RANDOM] ");
Serial.println(random);
#endif

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
#ifdef DEBUG_RANDOM
Serial.print("[DEBUG RANDOM] ERROR");
#endif
        }
   }

    return res;
}

