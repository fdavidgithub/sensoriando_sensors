/* 
 * 
 */
#ifndef ESPNOW_H
#define ESPNOW_H

#include <Esp.h>
#include <FS.h>
#include <SimpleEspNowConnection.h>
#include <sensoriando.h>

#include "gpio.h"


/*
 * MACROS
 */
#define DEBUG_ESPNOW
#define CONFIGFILE  "/espnow.conf"
#define PAIRTIME    60


/*
 * Global Variables
 */
typedef SimpleEspNowConnection espnow_connection;

typedef void (*MessageFunction)(uint8_t *, const uint8_t *, size_t);
typedef void (*NewGatewayAddressFunction)(uint8_t *, String);
typedef void (*PairedFunction)(uint8_t *, String);
typedef void (*ConnectedFunction)(uint8_t *, String);
typedef void (*SendErrorFunction)(uint8_t *);
typedef void (*SendDoneFunction)(uint8_t *);
typedef void (*PairingFinishedFunction)(void);


/*
 * Prototypes
 */
byte espnow_init(espnow_connection *, MessageFunction, \
                    SendErrorFunction, SendDoneFunction,
                    NewGatewayAddressFunction, ConnectedFunction, \
                    PairedFunction, PairingFinishedFunction);
void espnow_reset();
byte espnow_connected(espnow_connection *);
byte espnow_pair(espnow_connection *);
byte espnow_send(espnow_connection *, SensoriandoSensorDatum *);
bool espnow_writeconf(String);
bool espnow_readconf(String *);

#endif


