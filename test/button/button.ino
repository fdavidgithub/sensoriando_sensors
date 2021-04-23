#include "gpio.h"

/*
 * MACRO
 */
#define DEBOUNCE  10

void setup()
{
    Serial.begin(115200);
    pinMode(GPIO_BUTTON, INPUT);
}

void loop()
{
    static byte state = 0;
    static byte counter = 0;
    byte button = !digitalRead(GPIO_BUTTON);
    
    if ( button ) {
        if ( !state ) {
          Serial.print("Pressed: #");Serial.println(counter);
          state = 1;
          counter++;
        } 
    } else {
      state = 0;
    }

    delay(DEBOUNCE);
}
