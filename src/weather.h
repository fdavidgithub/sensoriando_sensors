/* 
 * Read temperature and humility sensor  
 * Libraries (Sketch >> Include Library >> Manage Libraies)  
 *  SimpleDHT
 */
#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <SimpleDHT.h>
#include <sensoriandoData.h>
#include "gpio.h"


/*
 * MACROS
 */
//#define DEBUG_WEATHER
#define KELVIN(x)       ((x) + 273.15)

//Uniques for each hardware compiled
#define WEATHER_UUID    "e8989226-9475-4c09-8e53-77f759223a6d"  //uuid of sensor
#define WEATHER_LEN     2                                       //Account of sensors  
#define TEMPERATURE_ID  5                                       //id of temperature
#define HUMIDITY_ID     6                                       //id of himidity


/*
 * Prototypes
 */
int weather_init(SensoriandoSensorDatum **);
int weather_read(SensoriandoSensorDatum **);

#endif

