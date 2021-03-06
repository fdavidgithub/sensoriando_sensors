/*
 * GPIO Map
 */
#ifndef GPIO_H
#define GPIO_H

#define GPIO_RESET   D0
#define GPIO_LED     LED_BUILTIN


/*
 * For the I²C bus
 *                  | Arduino UNO   | Arduino MEGA  | ESP-12E   | ESP-01 
 *-----------------------------------------------------------------------
 * SDA: Data line   | A4            | 22            | GPIO04    | GPIO0 
 * SCL: Clock line  | A5            | 21            | GPIO05    | GPIO2
 */
#define GPIO_SDA     SDA
#define GPIO_SCL     SCL


/*
 * Weather
 * DFRobot DHT11
 * Interface: Digital
 */
#define GPIO_WEATHER    0   //GPIO ESP8266 (pin D3 ESP-12E)

#endif

