 /*
   Read sensors and send to Sensoriando_Hub

   Build with IDE Arduino 1.8.12
   Addicional Board Manager (File >> Preferences)
   https://dl.espressif.com/dl/package_esp32_index.json
   http://arduino.esp8266.com/stable/package_esp8266com_index.json

   REQUIREMENT COMPILE
   1x Board NodeMCU
      Tools>>Board>>Board Manager (esp8266 by ESP8266 Community v2.7.1)

   NodeMCU ESP8266 Tools>>Board (NodeMCU 1.0)
   Module ESP8266  Tools>>Board (Generic 8266 Module)   

   Libraries (Sketch >> Include Library >> Manage Libraies)
      Sensoriando             by Francis David      v1.0.0
      SimpleEspNowConnection  by Erich O.Pintar     v1.2.0
      SimpleDHT               by Winlin             v1.0.15
      WifiManager             by tzapu              v2.0.16


*/
#include <Arduino.h>
#include <sensoriando.h>

#include "src/log.h"
#include "src/gpio.h"
#include "src/messages.h"
#include "src/wifi.h"
#include "src/espnow.h"
#include "src/settings.h"

/*
 *  MACROS
 */
#define DEBUG
#define DIGITALREAD     !digitalRead

#ifdef DEBUG 
#define LOGGER(string, ...)       logargs("MAIN", string, ##__VA_ARGS__)
#else
#define LOGGER(string, ...)       do {} while(0)
#endif

//Operation mode
#define NOSENSOR        0x00    //Random values
#define ENVIRONMENT     0x01    //Temperature and humidity of air

//Configure module
#define DEBOUNCE        300
#define TIMERESET       5000   //default 5000 -> 5 seconds in miliseconds
#define TIMEPAIR        500
#define WDT             3000
#define TRYSEND         3

//Need send this block to compile param
#define MODULE          ENVIRONMENT


/* 
 * Set Module
 */
#if MODULE == NOSENSOR
#include "src/random.h"
int (*sensor_init)(SensoriandoSensorDatum **, char *) = random_init;
int (*sensor_read)(SensoriandoSensorDatum **, long *) = random_read;
#endif

#if MODULE == ENVIRONMENT
#include "src/environment.h"
int (*sensor_init)(SensoriandoSensorDatum **, char *) = environment_init;
int (*sensor_read)(SensoriandoSensorDatum **, long *) = environment_read;
#endif


/*
 * GlobalVariable
 */
enum ConnectMode {NONE, ESPNOW, WIFI};

//Wifi
wifi_connection ConnectWifi;
WiFiClient EspClient;
SensoriandoObj Broker(EspClient);
uint8_t MacAddress[6];

//EspNow
espnow_connection ConnectEsp(SimpleEspNowRole::CLIENT);

//BOTH (wifi and esp)
SensoriandoSensorDatum *datum;
SensoriandoSensorDatum *last_datum;
int ConnectInUse = NONE;
int ErrSendCounter = 0;


/*
 * Prototypes
 */
int ReadSensor(long *);
byte DatumSend(SensoriandoSensorDatum *);
void SetConnWifi(char*, char*);

void OnSendDone(uint8_t *);
void OnSendError(uint8_t *);
//void OnPaired(uint8_t *, String);
void OnNewGatewayAddress(uint8_t *, String);
//void OnConnected(uint8_t *, String);
//void OnMessage(uint8_t *, const uint8_t *, size_t);
void OnPairingFinished();


/*
 *  Main Program
 */
void setup()
{
  File settings_file;
  char* uuid = NULL; 
  char* brokerUser = NULL;
  char* brokerPass = NULL;
  
  //Watch Dog Timer
  ESP.wdtEnable(WDT);

  //Setting pins
  pinMode(GPIO_SELECT, INPUT);
  pinMode(GPIO_ERROR, OUTPUT);
  pinMode(GPIO_CONFIG, OUTPUT);
  
  digitalWrite(GPIO_ERROR, 0);
  digitalWrite(GPIO_CONFIG, 1);
 
  //Serial baudrate
  Serial.begin(115200);
  delay(1000);
  LOGGER("DEBUG MODE");


  /*
   * Environment settings 
   */
  settings_open(settings_file);
    
  if ( settings_file ) {
    uuid = settings_string(settings_file, "uuid");
   
    if ( !uuid ) { 
      LOGGER("Environment UUID is empty");
    }

    brokerUser = settings_string(settings_file, "username");
   
    if ( !brokerUser ) { 
      LOGGER("Environment broker username is empty");
    }

    brokerPass = settings_string(settings_file, "password");
   
    if ( !brokerPass ) { 
      LOGGER("Environment broker password is empty");
    }

    settings_close(settings_file);
  } else {
    LOGGER("ATTENTION: Environment file not found");
  }


  /* 
   * Settings
   */
  if ( uuid ) {
    LOGGER("UUID %s", uuid);
  } else {
    digitalWrite(GPIO_ERROR, 1);
    delay(TIMERESET);
    ESP.reset();
  }

  if ( brokerUser ) {
    LOGGER("Broker user %s", brokerUser);
  } else {
    digitalWrite(GPIO_ERROR, 1);
    delay(TIMERESET);
    ESP.reset();
  }

  if ( brokerPass ) {
    LOGGER("Broker pass %s", brokerPass);
  } else {
    digitalWrite(GPIO_ERROR, 1);
    delay(TIMERESET);
    ESP.reset();
  }

  
  /*
   * Connections 
   *
  //1st Try ESPNow
  LOGGER("Try EspNow Connection...");

  if ( espnow_init(&ConnectEsp, &OnSendError, &OnSendDone, \
                   &OnNewGatewayAddress, &OnPairingFinished) ) {
    ConnectInUse = ESPNOW;
  } else {*/
    LOGGER("Try Wifi Connection...");
    LOGGER("MAC Address: ");
    //LOGGER(WiFi.macAddress());

    SetConnWifi(brokerUser, brokerPass);
  //}

  //Sensor
  if ( ! sensor_init(&datum, uuid) ) {
    ESP.reset();
  }

  last_datum = (SensoriandoSensorDatum *)malloc(sizeof(datum));

  //Connection
  if ( ! ConnectInUse ) {
    LOGGER("Without Connection!");
    digitalWrite(GPIO_ERROR, 1);
  } else {
    LOGGER("Waiting for sensor...");
    digitalWrite(GPIO_ERROR, 0);
  }

  digitalWrite(GPIO_CONFIG, 0);
}

void loop()
{
  int sensors;
  static long config_elapsed;
  static long update_elapsed = millis();


  /*
   * Operation mode
   */
  if  ( DIGITALREAD(GPIO_SELECT) ) {
    if ( ((millis() - config_elapsed) > TIMEPAIR) && ConnectInUse ) {
      if ( espnow_pair(&ConnectEsp) ) {
        LOGGER("Pairing...");

        ConnectInUse = NONE;
        delay(DEBOUNCE);
      }
    }

    if ( (millis() - config_elapsed) > TIMERESET ) {
      LOGGER("Reseting...");

      espnow_reset();
      wifi_reset(&ConnectWifi);
      delay(DEBOUNCE);

      ESP.reset();
    }
  } else {
    config_elapsed = millis();
  }

  switch ( ConnectInUse ) {
    case ESPNOW:    ConnectEsp.loop(); 
                    break;
    case NONE:      ConnectEsp.loop();
                    break;
    case WIFI:      Broker.loop();
                    break;    
  }

#ifdef DEBUG
  if ( Serial.available() ) {
    switch (Serial.read()) {
      case 'r': ESP.reset(); break;
      case 'e': espnow_reset(); break;
      case 'w': wifi_reset(&ConnectWifi); break;
    }
  }
#endif


  /*
   * Publisher
   */
  sensors = ReadSensor(&update_elapsed);
  
  if ( sensors && ConnectInUse ) {
    switch ( ConnectInUse ) {
      case ESPNOW: if ( !espnow_connected(&ConnectEsp) ) {
          LOGGER("EspNow: Without connect, rebooting...");
          ESP.reset();
        }

        break;
      case WIFI: if ( !wifi_connected() ) {
          LOGGER("Wifi: Without connect, rebooting...");
          ESP.reset();
        }

        if ( !sensoriandoReconnect(&Broker, MacAddress) ) {
          LOGGER("Broker: Without connect, rebooting...");
          ESP.reset();
        }
        break;
      default: break;
    }

    LOGGER("Sending data from %d sensors...", sensors);

    for (int i = 0; i < sensors; i++) {
      LOGGER("id %d | value %f", (datum)[i].id, (datum)[i].value);

      if (last_datum[i].value == datum[i].value) {
        LOGGER("SOME VALUE, SKIP");
        continue;
      }
        
      if ( DatumSend(&((datum)[i])) ) {
        LOGGER("Sent datum from sensor %d", i);
        last_datum[i].value = datum[i].value;
        ErrSendCounter = 0;
      } else {
        LOGGER("Fail while sending datum from sensor %d", i);
        ErrSendCounter++;
      }
    }

    if ( (ErrSendCounter >= TRYSEND) ) {
      LOGGER("Resetting");
      ESP.reset();
    }
  }

}


/*
   functions
*/
void SetConnWifi(char* brokerUser, char* brokerPass)
{
  if ( wifi_init(&ConnectWifi, MacAddress) ) {
    ConnectInUse = WIFI;

    //Sensoriando
    if ( !sensoriandoInit(&Broker, MacAddress, brokerUser, brokerPass) ) {
        LOGGER("Broker do not init");
        digitalWrite(GPIO_ERROR, 1);
    } else {
        LOGGER("Broker connected");
        digitalWrite(GPIO_ERROR, 0);
    }
  }
}

byte DatumSend(SensoriandoSensorDatum *datum)
{
    byte res;

    switch ( ConnectInUse ) {
        case ESPNOW:    res = espnow_send(&ConnectEsp, datum);
                        break;
        case WIFI:      res = wifi_send(&Broker, datum);
                        break;
        default:        res = 0;
                        break;
    }

    digitalWrite(GPIO_ERROR, !res);
    LOGGER("Datum Send Result %d", res);

    return res;
}

int ReadSensor(long *elapsed)
{
    int res = 0;

    res = sensor_read(&datum, elapsed);
    if ( res ) {
        LOGGER("Sensor read...");
    }

#ifdef DEBUG
    for (int i = 0; i < res; i++) {
        LOGGER("Bytes sent: %i", sizeof(SensoriandoSensorDatum));
        LOGGER("UUID: %s", datum[i].uuid);
        LOGGER("stx=0x%02X, id=%d, value=%02f, etx=0x%02X", datum[i].stx, datum[i].id, datum[i].value, datum[i].etx);
    }
#endif
  
    return res;
}

void OnSendError(uint8_t* ad)
{
    LOGGER("OnSendError");
    //LOGGER(ConnectEsp.macToStr(ad));
}

void OnSendDone(uint8_t* ad)
{
    LOGGER("OnSendDone");
    //LOGGER(ConnectEsp.macToStr(ad));
}

void OnPairingFinished()
{
    LOGGER("OnPairingFinished");
}

void OnNewGatewayAddress(uint8_t *ga, String ad)
{
    ConnectEsp.setServerMac(ga);

    LOGGER("OnNewGatewayAddress: %i", sizeof(ad));
    LOGGER("Gateway: ");
    //  for (int i = 0; i < 6; i++) {
    //    Serial.print(ga[i], HEX); Serial.print(" ");
    //  }
    //  Serial.print("[DEBUG] MAC Address: "); Serial.println(ad);

    if ( !espnow_writeconf(ad) ) {
        LOGGER("SPIFFS: Error while write in file");
    } else {
        LOGGER("SPIFFS: Write in file");

        ConnectInUse = ESPNOW;
        //wifi_reset(&ConnectWifi);
        delay(DEBOUNCE);
        ESP.reset();
    }
}

/*
  void OnConnected(uint8_t *ga, String ad)
  {
  LOGGER("onConnected %s", ad);
  }

  void OnMessage(uint8_t* ad, const uint8_t* message, size_t len)
  {
  LOGGER("OnMessage from %s", (char *)ad);
  LOGGER((char *)message);
  }

  void OnPaired(uint8_t *ga, String ad)
  {
  LOGGER("OnPaired, server %s paired", ad);

  ConnectInUse = ESPNOW;
  ConnectEsp.endPairing();
  }

*/
