/* 
 * Read sensors and send to Sensoriando_Hub
 * 
 * Build with IDE Arduino 1.8.12
 * Addicional Board Manager (File >> Preferences)
 * https://dl.espressif.com/dl/package_esp32_index.json
 * http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *
 * REQUIREMENT COMPILE
 * 1x Board NodeMCU
 *    Tools>>Board>>Board Manager (esp8266 by ESP8266 Community v2.7.1)
 *    Tools>>Board (NodeMCU 1.0)
 *      
 * Libraries (Sketch >> Include Library >> Manage Libraies)  
 *    Sensoriando       by Francis David      v1.0.0  
 *    SimplesEspNow     by                    v
 *    WifiManager       by                    v
 *    
 *
 */
#include <sensoriando.h>

#include "src/gpio.h"
#include "src/messages.h"
#include "src/wifi.h"
#include "src/espnow.h"


/*
 * MACROS
 */
//#define DEBUG 

//Operation mode
#define NOSENSOR    0x00    //Random values
#define WEATHER     0x01    //Temperature and humidity of air

#define UPDATEELAPSED   1000
#define DEBOUNCE        300
#define TIMERESET       5000   //default 5000 -> 5 seconds in miliseconds
#define TIMEPAIR        300
#define WDT             3000

//Current module
#define MODULE  NOSENSOR

#if MODULE == WEATHER
  #include "src/weather.h"
#endif


/*
 * GlobalVariable
 */
wifi_connection ConnectWifi;
WiFiClient EspClient;
SensoriandoObj Broker(EspClient);

//for esp
espnow_connection ConnectEsp(SimpleEspNowRole::CLIENT);

//both (wifi and esp)
SensoriandoSensorDatum *datum;
enum ModeOper{None, EspNow, Wifi};
int ModeUse=None;


/*
 * Prototypes
 */
int ReadSensor();
int RandomSensor();
byte DatumSend(SensoriandoSensorDatum *);

void OnSendDone(uint8_t *);
void OnSendError(uint8_t *);
void OnPaired(uint8_t *, String);
void OnNewGatewayAddress(uint8_t *, String);
void OnConnected(uint8_t *, String);
void OnMessage(uint8_t *, const uint8_t *, size_t);
void OnPairingFinished();


/*
 * Main Program 
 */
void setup()
{     
    ESP.wdtEnable(WDT);       
    Serial.begin(115200);
    
    pinMode(GPIO_RESET, INPUT);

#ifdef DEBUG
Serial.println();
Serial.print("[DEBUG] Client Address: ");
Serial.println(WiFi.macAddress());
#endif

    //Connection

#ifdef DEBUG
Serial.println("[DEBUG] Try EspNow Connection...");
#endif
    
    if ( espnow_init(&ConnectEsp, &OnMessage, \
            &OnSendError, &OnSendDone, \
            &OnNewGatewayAddress, &OnConnected, \
            &OnPaired, &OnPairingFinished) ) {     

        ModeUse = EspNow;
    } else { 

#ifdef DEBUG
Serial.println("[DEBUG] Try Wifi Connection...");
#endif

        if ( wifi_init(&ConnectWifi) ) {
            ModeUse = Wifi; 

            //Sensoriando
            if ( !sensoriandoInit(&Broker) ) {
#ifdef DEBUG
Serial.println("[DEBUG] Broker do not init");
#endif
            } else {
#ifdef DEBUG
Serial.println("[DEBUG] Broker connected");
#endif
            }    
        } else {
#ifdef DEBUG
Serial.println("[DEBUG] Connection None");
#endif
            ModeUse = None;      
        }        
    }
    
    //Sensor
    #if MODULE == WEATHER
        if ( ! weather_init(&datum) ) {
            ESP.reset();
        }
    #else
        if ( ! RandomSensor() ) {
            ESP.reset();
        }
    #endif
    
#ifdef DEBUG
Serial.println("[DEBUG] Waiting for sensor...");
#endif

}

void loop()
{
    int i;
    int sensors;
    static long updateelapsed=millis();
    static long resetelapsed;
    static long pairelapsed;
        
    ConnectEsp.loop();
    
    if  ( digitalRead(GPIO_RESET) ) {
        if ( ((millis()-pairelapsed) > TIMEPAIR) && espnow_pair(&ConnectEsp) ) {
#ifdef DEBUG
Serial.println("[DEBUG] Pairing...");
#endif
            delay(DEBOUNCE);
        }
        
        if ( (millis()-resetelapsed) > TIMERESET ) {       
#ifdef DEBUG
Serial.println("[DEBUG] Reseting...");
#endif
            espnow_reset();
            wifi_reset(&ConnectWifi);
            delay(DEBOUNCE);
            
            ESP.reset();           
        } 
    } else {
        resetelapsed=millis();
        pairelapsed=millis();
    }

#ifdef DEBUG
if ( Serial.available() && (Serial.read() == 'r') ) {
  ESP.reset();
}
#endif
    
    if ( (ModeUse != None) && (millis()-updateelapsed > UPDATEELAPSED) ) {
        updateelapsed=millis();
        sensors = ReadSensor();

#ifdef DEBUG
Serial.printf("[DEBUG] Sending data from %d sensors...\n", sensors);
#endif

        for (i=0; i<sensors; i++) {
#ifdef DEBUG
Serial.printf("[DEBUG] id %d | value %f\n", (datum)[i].id, (datum)[i].value);
digitalWrite(GPIO_LED, 0);
#endif
            
            if ( DatumSend(&((datum)[i])) ) {
#ifdef DEBUG
Serial.printf("[DEBUG] Sent datum from sensor %d\n", i);
#endif
            } else {

#ifdef DEBUG
Serial.println("[DEBUG] Fail while sending datum from sensor!");
#endif
            }
        }

#ifdef DEBUG
digitalWrite(GPIO_LED, 1);
#endif

    }
}


/*
 * functions 
 */
byte DatumSend(SensoriandoSensorDatum *datum)
{    
    byte res;

    switch ( ModeUse ) {
        case EspNow:  res = espnow_send(&ConnectEsp, datum);
                      break;
        case Wifi:    res = wifi_send(&Broker, datum);
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
    int res=0;
 
    #if MODULE == WEATHER
        res=weather_read(&datum);

#ifdef DEBUG
Serial.println(res);
#endif

    #else
        datum->stx = STX;
        datum->id = 0;
        datum->dt = NULL;
        strcpy(datum->uuid, "00000000-0000-0000-0000-000000000000"); //UUID not existed
        datum->value = random(1,100)/PI;  
        datum->etx = ETX;
        
        res = 1;    
    #endif

#ifdef DEBUG
for (int i=0; i<res; i++) {
Serial.print("[DEBUG] Bytes sent: ");
Serial.println(sizeof(SensoriandoSensorDatum));
Serial.print("[DEBUG] UUID: ");Serial.println(datum[i].uuid);
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

  delay(1000);
  ESP.reset();
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
for (int i=0; i<6; i++) {
  Serial.print(ga[i], HEX);Serial.print(" ");
} 
Serial.println("");
Serial.print("[DEBUG] Address: ");Serial.println(ad);
#endif

    if ( !espnow_writeconf(ad) ) {
#ifdef DEBUG
Serial.println("[DEBUG] SPIFFS: Error while write in file");
#endif   
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
Serial.print("[DEBUG] OnMessage from ");Serial.println((char *)ad);
Serial.println((char *)message);
#endif
}
