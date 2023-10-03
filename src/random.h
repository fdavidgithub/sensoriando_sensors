/* 
 * Read random generator sensor  
 * Libraries (Sketch >> Include Library >> Manage Libraies)  
 *
 */
#ifndef RANDOM_H
#define RAONDAM_H

#include <Arduino.h>
#include <sensoriandoData.h>


/*
 * MACROS
 */
//#define DEBUG_RANDOM
#define UPDATEELAPSED   2000

//Uniques for each hardware compiled
#define RANDOM_LEN     1                                       //Account of sensors  
#define RANDOM_ID  0                                       //id of random


/*
 * Prototypes
 */
int random_init(SensoriandoSensorDatum **, char *);
int random_read(SensoriandoSensorDatum **, long *);

#endif

