/*
   Read sensors and send to Sensoriando_Hub

   Build with IDE Arduino 1.8.12
   Addicional Board Manager (File >> Preferences)
   https://dl.espressif.com/dl/package_esp32_index.json
   http://arduino.esp8266.com/stable/package_esp8266com_index.json

   REQUIREMENT COMPILE
   1x Board NodeMCU
      Tools>>Board>>Board Manager (esp8266 by ESP8266 Community v2.7.1)
      Tools>>Board (NodeMCU 1.0)

   Libraries (Sketch >> Include Library >> Manage Libraies)
      Sensoriando       by Francis David      v1.0.0
      SimplesEspNow     by                    v
      WifiManager       by                    v


*/
#include <sensoriando.h>

#include "src/gpio.h"
#include "src/messages.h"
#include "src/wifi.h"
#include "src/espnow.h"


/*
   MACROS
*/
//#define DEBUG
#define DIGITALREAD   !digitalRead

//Operation mode
#define NOSENSOR    0x00    //Random values
#define WEATHER     0x01    //Temperature and humidity of air

//Configure module
#define UPDATEELAPSED   1500
#define DEBOUNCE        300
#define TIMERESET       5000   //default 5000 -> 5 seconds in miliseconds
#define TIMEPAIR        500
#define WDT             3000
#define TRYSEND         3

//Need send this block to compile param
#define UUID    "e8989226-9475-4c09-8e53-77f759223a6d" 
#define MODULE  WEATHER

#if MODULE == WEATHER
#include "src/weather.h"
#endif


/*
   GlobalVariable
*/
enum ConnectMode {NONE, ESPNOW, WIFI};

//Wifi
wifi_connection ConnectWifi;
WiFiClient EspClient;
SensoriandoObj Broker(EspClient);

//EspNow
espnow_connection ConnectEsp(SimpleEspNowRole::CLIENT);

//BOTH (wifi and esp)
SensoriandoSensorDatum *datum;
int ConnectInUse = NONE;
int ErrSendCounter = 0;


/*
   Prototypes
*/
int ReadSensor();
int RandomSensor();
byte DatumSend(SensoriandoSensorDatum *);
void SetConnWifi();

void OnSendDone(uint8_t *);
void OnSendError(uint8_t *);
void OnPaired(uint8_t *, String);
void OnNewGatewayAddress(uint8_t *, String);
void OnConnected(uint8_t *, String);
void OnMessage(uint8_t *, const uint8_t *, size_t);
void OnPairingFinished();


/*
   Main Program
*/
void setup()
{
  ESP.wdtEnable(WDT);

  //Setting pins
  pinMode(GPIO_CONFIG, INPUT);

  //Serial baudrate
#ifdef DEBUG  
  Serial.begin(115200);
  Serial.println("[DEBUG MODE]");
#endif

  //Connection (1st Try ESPNow)
#ifdef DEBUG
  Serial.println("[DEBUG] Try EspNow Connection...");
#endif

  if ( espnow_init(&ConnectEsp, &OnMessage, \
                   &OnSendError, &OnSendDone, \
                   &OnNewGatewayAddress, &OnConnected, \
                   &OnPaired, &OnPairingFinished) ) {

    ConnectInUse = ESPNOW;
  }

  if ( ! ConnectInUse ) {
#ifdef DEBUG
    Serial.println("[DEBUG] Try Wifi Connection...");
#endif

#ifdef DEBUG
    Serial.println();
    Serial.print("[DEBUG] MAC Address: ");
    Serial.println(WiFi.macAddress());
#endif

    SetConnWifi();
  }

  //Sensor
#if MODULE == WEATHER
  if ( ! weather_init(&datum, UUID) ) {
    ESP.reset();
  }
#else
  if ( ! RandomSensor() ) {
    ESP.reset();
  }
#endif

  if ( ! ConnectInUse ) {
#ifdef DEBUG
    Serial.println("[DEBUG] Without Connection!");
#endif
  } else {
#ifdef DEBUG
    Serial.println("[DEBUG] Waiting for sensor...");
#endif
  }

}

void loop()
{
  int i;
  int sensors;
  static long updateelapsed = millis();
  static long resetelapsed;
  static long pairelapsed;

  if  ( DIGITALREAD(GPIO_CONFIG) ) {
    if ( ((millis() - pairelapsed) > TIMEPAIR) && espnow_pair(&ConnectEsp) ) {
#ifdef DEBUG
      Serial.println("[DEBUG] Pairing...");
#endif
      ConnectInUse = NONE;
      delay(DEBOUNCE);
    }

    if ( (millis() - resetelapsed) > TIMERESET ) {
#ifdef DEBUG
      Serial.println("[DEBUG] Reseting...");
#endif

      espnow_reset();
      wifi_reset(&ConnectWifi);
      delay(DEBOUNCE);

      ESP.reset();
    }
  } else {
    resetelapsed = millis();
    pairelapsed = millis();
  }

  if ( (ConnectInUse == ESPNOW) || (ConnectInUse == NONE) ) {
    ConnectEsp.loop();
  }

#ifdef DEBUG
  if ( Serial.available() && (Serial.read() == 'r') ) {
    ESP.reset();
  }
#endif

  if ( (ConnectInUse) && (millis() - updateelapsed > UPDATEELAPSED) ) {

    switch ( ConnectInUse ) {
      case ESPNOW: if ( ! espnow_connected(&ConnectEsp) ) {
#ifdef DEBUG
  Serial.print("[DEBUG] EspNow: Without connect, rebooting...");
#endif
              ESP.reset();
              }
              
              break;
      case WIFI: if ( ! wifi_connected() ) {
#ifdef DEBUG
  Serial.print("[DEBUG] Wifi: Without connect, rebooting...");
#endif
              ESP.reset();
            }
            
            break;
      default: break;
    }
    
    updateelapsed = millis();
    sensors = ReadSensor();

#ifdef DEBUG
    Serial.printf("[DEBUG] Sending data from %d sensors...\n", sensors);
#endif

    for (i = 0; i < sensors; i++) {
#ifdef DEBUG
      Serial.printf("[DEBUG] id %d | value %f\n", (datum)[i].id, (datum)[i].value);
#endif

      if ( DatumSend(&((datum)[i])) ) {
#ifdef DEBUG
        Serial.printf("[DEBUG] Sent datum from sensor %d\n", i);
#endif
        ErrSendCounter = 0;
      } else {

#ifdef DEBUG
        Serial.println("[DEBUG] Fail while sending datum from sensor!");
#endif
        ErrSendCounter++;
      }
    }

    if ( (ErrSendCounter >= TRYSEND) ) {
Serial.println("Resetting");
          ESP.reset();  
    }

  }
}


/*
   functions
*/
void SetConnWifi()
{
    uint8_t mac[6];
    
    if ( wifi_init(&ConnectWifi) ) {
      ConnectInUse = WIFI;
      
      //Sensoriando
      WiFi.macAddress(mac);

      if ( !sensoriandoInit(&Broker, mac) ) {
#ifdef DEBUG
        Serial.println("[DEBUG] Broker do not init");
#endif
      } else {
#ifdef DEBUG
        Serial.println("[DEBUG] Broker connected");
#endif
      }
    }  
}

byte DatumSend(SensoriandoSensorDatum *datum)
{
  byte res;

  switch ( ConnectInUse ) {
    case ESPNOW:  res = espnow_send(&ConnectEsp, datum);
      break;
    case WIFI:    res = wifi_send(&Broker, datum);
      break;
    default:      res = 0;
      break;
  }

#ifdef DEBUG
  Serial.printf("[DEBUG] Datum Send Result %d\n", res);
#endif

  return res;
}

int RandomSensor()
{
  datum = (SensoriandoSensorDatum *)malloc(sizeof(SensoriandoSensorDatum));
  return datum != NULL;
}

int ReadSensor()
{
  int res = 0;

#if MODULE == WEATHER
  res = weather_read(&datum);
#else
  datum->stx = STX;
  datum->id = 0;
  datum->dt = NULL;
  strcpy(datum->uuid, "00000000-0000-0000-0000-000000000000"); //UUID not existed
  datum->value = random(1, 100) / PI;
  datum->etx = ETX;

  res = 1;
#endif

#ifdef DEBUG
  for (int i = 0; i < res; i++) {
    Serial.print("[DEBUG] Bytes sent: ");
    Serial.println(sizeof(SensoriandoSensorDatum));
    Serial.print("[DEBUG] UUID: "); Serial.println(datum[i].uuid);
    Serial.printf("stx=0x%02X, id=%d, value=%02f, etx=0x%02X\n", datum[i].stx, datum[i].id, datum[i].value, datum[i].etx);
  }
#endif

  return res;
}

void OnSendError(uint8_t* ad)
{
#ifdef DEBUG
  Serial.println("[DEBUG] OnSendError " + ConnectEsp.macToStr(ad));
#endif
}

void OnSendDone(uint8_t* ad)
{
#ifdef DEBUG
  Serial.println("[DEBUG] OnSendDone " + ConnectEsp.macToStr(ad));
#endif
}

void OnPairingFinished()
{
#ifdef DEBUG
  Serial.println("[DEBUG] OnPairingFinished");
#endif
}

void OnPaired(uint8_t *ga, String ad)
{
#ifdef DEBUG
  Serial.println("[DEBUG] OnPaired, server '" + ad + " paired! ");
#endif

  ConnectEsp.endPairing();
}

void OnNewGatewayAddress(uint8_t *ga, String ad)
{
  ConnectEsp.setServerMac(ga);

#ifdef DEBUG
  Serial.println(sizeof(ad));
  Serial.println("[DEBUG] OnNewGatewayAddress");
  Serial.print("Gateway: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(ga[i], HEX); Serial.print(" ");
  }
  Serial.println("");
  Serial.print("[DEBUG] MAC Address: "); Serial.println(ad);
#endif

  if ( !espnow_writeconf(ad) ) {
#ifdef DEBUG
    Serial.println("[DEBUG] SPIFFS: Error while write in file");
#endif
  } else {
#ifdef DEBUG
    Serial.print("[DEBUG] SPIFFS: Write in file");
#endif

    wifi_reset(&ConnectWifi);
    delay(DEBOUNCE);
    ESP.reset();
  }
}

void OnConnected(uint8_t *ga, String ad)
{
#ifdef DEBUG
  Serial.println("[DEBUG] onConnected"); Serial.print(ad);
#endif
}

void OnMessage(uint8_t* ad, const uint8_t* message, size_t len)
{
#ifdef DEBUG
  Serial.print("[DEBUG] OnMessage from "); Serial.println((char *)ad);
  Serial.println((char *)message);
#endif
}
