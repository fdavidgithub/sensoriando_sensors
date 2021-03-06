/* 
 * 
 */
#ifndef WIFI_H
#define WIFI_H

#include <ESP8266WiFi.h>   
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <sensoriando.h>


/*
 * MACROS
 */
//#define DEBUG_WIFI
#define PREFIXAPMODE  "Sensoriando"


/*
 * Global Variables
 */
typedef WiFiManager wifi_connection;


/*
 * Prototypes
 */
void wifi_reset(wifi_connection *);
byte wifi_init(wifi_connection *);
byte wifi_send(SensoriandoObj *, SensoriandoSensorDatum *);
byte wifi_connected();

#endif
