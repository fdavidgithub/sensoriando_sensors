/* 
 * Read temperature and humility sensor  
 * Libraries (Sketch >> Include Library >> Manage Libraies)  
 *  SimpleDHT
 */
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <Arduino.h>
#include <SimpleDHT.h>
#include <sensoriandoData.h>
#include "gpio.h"
#include "log.h"


/*
 * MACROS
 */
#define DEBUG_ENVIRONMENT
#define KELVIN(x)       ((x) + 273.15)
#define UPDATEELAPSED   1500

//Uniques for each hardware compiled
#define ENVIRONMENT_LEN     2                                       //Account of sensors  
#define TEMPERATURE_ID  6                                       //id of temperature
#define HUMIDITY_ID     16                                       //id of himidity

#ifdef DEBUG_ENVIRONMENT
#define LOGGER_ENV(string, ...)       logargs("ENVIRONMENT", string, ##__VA_ARGS__)
#else
#define LOGGER_ENV(string, ...)       do {} while(0)
#endif

/*
 * Prototypes
 */
int environment_init(SensoriandoSensorDatum **, char *);
int environment_read(SensoriandoSensorDatum **, long *);

#endif

