#include "button.h"

int button_init(SensoriandoSensorDatum **datum, char *uuid)
{
    byte res;

#ifdef DEBUG_WEATHER
Serial.print(WEATHER_LEN * sizeof(*datum));
Serial.println("[DEBUG BUTTON] Button init...");
#endif

    *datum = (SensoriandoSensorDatum *)malloc(BUTTON_LEN * sizeof(SensoriandoSensorDatum));
    res = *datum != NULL;

    if ( res ) {
#ifdef DEBUG_BUTTON
Serial.println("[DEBUG BUTTON] Memory allocated...");
Serial.print("Size struct: ");Serial.println(sizeof(SensoriandoSensorDatum));
Serial.print("Size alloc: ");Serial.println(sizeof(*datum));
Serial.print("Size array: ");Serial.println(sizeof(*datum) / sizeof(SensoriandoSensorDatum));
#endif

        (*datum)->stx = STX;
        (*datum)->dt = NULL;
        (*datum)->value = NULL;
        (*datum)->id = STATE_ID;
        (*datum)->etx = ETX;
        strcpy((*datum)->uuid, uuid);
    }

    pinMode(GPIO_BUTTON, INPUT);

    return res;
}

int button_read(SensoriandoSensorDatum **datum) 
{
    byte button;  
    int res=0;

    button = DIGITALREAD(GPIO_BUTTON);

    if ( button ) {
        if ( !((*datum)->value) ) {
#ifdef DEBUG_BUTTON
Serial.println("[DEBUG BUTTON] Pressed");
#endif
            res = 1;
            (*datum)->value = res;

            delay(DEBOUNCE);
        }        
    } else {
        (*datum)->value = res;
    }

    return res;
}

