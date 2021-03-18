/*
 * GPIO Map
 */
#ifndef GPIO_H
#define GPIO_H

#define GPIO_CONFIG     0           //GPIO ESP8266 (NODEMCU Pin D3)
#define GPIO_PAIR       BUILTIN_LED //GPIO ESP8266 (NODEMCU Pin D4) 


/*
 * For the I²C bus
 *                  | Arduino UNO   | Arduino MEGA  | ESP-12E   | ESP-01 
 *-----------------------------------------------------------------------
 * SDA: Data line   | A4            | 22            | GPIO04    | GPIO0 
 * SCL: Clock line  | A5            | 21            | GPIO05    | GPIO2
 */
#define GPIO_SDA        SDA
#define GPIO_SCL        SCL


/*
 * Data
 * Interface: Digital
 */
#define GPIO_WEATHER    2   //GPIO ESP8266 (NODEMCU pin D4)

#endif

