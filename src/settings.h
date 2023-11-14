/* 
 * 
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <string.h>

#include "log.h"


/*
 * MACROS
 */
#define DEBUG_SETTINGS
#define CONFIGFILE  "/settings.conf"

#ifdef DEBUG_SETTINGS
#define LOGGER_SET(string, ...)       logargs("SETTINGS", string, ##__VA_ARGS__)
#else
#define LOGGER_SET(string, ...)       do {} while(0)
#endif


/*
 * Global Variables
 */


/*
 * Prototypes
 */
void settings_open(File&);
char* settings_string(File&, char*);
void settings_close(File&);

#endif


