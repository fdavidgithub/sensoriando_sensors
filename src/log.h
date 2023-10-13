/* 
 * Read random generator sensor  
 * Libraries (Sketch >> Include Library >> Manage Libraies)  
 *
 */
#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include <stdarg.h>


/*
 * MACROS
 */
//#define DEBUG_LOG


/*
 * Prototypes
 */
void logger(char *, char *);
void logger(char *, int);
void logger(char *, double);

void logargs(char *, char*, ...);
#endif

