/* 
 * Read random generator sensor  
 * Libraries (Sketch >> Include Library >> Manage Libraies)  
 *
 */
#ifndef RANDOM_H
#define RANDOM_H

#include <Arduino.h>
#include <sensoriandoData.h>
#include "gpio.h"
#include "log.h"

/*
 * MACROS
 */
#define DEBUG_RANDOM
#define UPDATEELAPSED               2000

#ifdef DEBUG_RANDOM
#define LOGGER_RAN(string, ...)     logargs("RANDOM", string, ##__VA_ARGS__)
#else
#define LOGGER_RAN(string, ...)     do {} while(0)
#endif

 
//Uniques for each hardware compiled
#define RANDOM_LEN  1                                       //Account of sensors  
#define RANDOM_ID   0                                       //id of random


/*
 * Prototypes
 */
int random_init(SensoriandoSensorDatum **, char *);
int random_read(SensoriandoSensorDatum **, long *);

#endif

