/* 
 * Read state of button  
 * Libraries (Sketch >> Include Library >> Manage Libraies)  
 *  
 */
#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include <sensoriandoData.h>
#include "gpio.h"


/*
 * MACROS
 */
#define DEBUG_BUTTON
#define DEBOUNCE        100
#define DIGITALREAD     !digitalRead
 
//Uniques for each hardware compiled
#define BUTTON_LEN  1                                       //Account of sensors
#define STATE_ID    5                                       //id of state


/*
 * Prototypes
 */
int button_init(SensoriandoSensorDatum **, char *);
int button_read(SensoriandoSensorDatum **);

#endif

