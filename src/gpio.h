/*
 * GPIO Map
 */
#ifndef GPIO_H
#define GPIO_H

#define GPIO_PAIR    D0
#define GPIO_LED     LED_BUILTIN


/*
 * For the I²C bus
 *                  | Arduino UNO   | Arduino MEGA  | ESP-12E    | ESP-01 
 *-----------------------------------------------------------------------
 * SDA: Data line   | A4            | 22            | GPIO21    | GPIO0 
 * SCL: Clock line  | A5            | 21            | GPIO22    | GPIO2
 */
#define GPIO_SDA     SDA
#define GPIO_SCL     SCL


/*
 * Weather
 */

  
#endif

