/*
 * GPIO Map
 */
#ifndef GPIO_H
#define GPIO_H


/*
 * Pins for the I²C bus
 *                  | Arduino UNO   | Arduino MEGA  | ESP-12E   | ESP-01 
 *-----------------------------------------------------------------------
 * SDA: Data line   | A4            | 22            | GPIO04    | GPIO0 
 * SCL: Clock line  | A5            | 21            | GPIO05    | GPIO2
 */
#define GPIO_SDA        SDA
#define GPIO_SCL        SCL


/*
 * Pins for analog
 *                  | Arduino UNO   | Arduino MEGA  | ESP-12E   | ESP-01 
 *-----------------------------------------------------------------------
 * ADC              | A0            | A0            | A0        | --- 
 */
#define GPIO_ADC        A0


/*
 * Digital
 */
#define GPIO_WAKE       16              //GPIO ESP3266 (NODEMCU pin D0)
#define GPIO_ERROR      12              //GPIO ESP8266 (NODEMCU pin D6)
#define GPIO_DATA       2               //GPIO ESP8266 (NODEMCU pin D4)
#define GPIO_CONFIG     13              //GPIO ESP8266 (NODEMCU pin D7)
#define GPIO_SELECT     0               //GPIO ESP8266 (NODEMCU pin D3) 


#endif

