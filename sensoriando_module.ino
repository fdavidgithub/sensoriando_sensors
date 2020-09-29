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
 *
 */
#include <sensoriandoData.h>
#include <SimpleEspNowConnection.h>
#include "src/gpio.h"
#include "src/messages.h"


/*
 * MACROS
 */
#define DEBUG 
#define MODULE  0x01

#define UPDATEELAPSED   1000
#define DEBOUNCE        300

#define NOSENSOR    0x00    //Random values
#define WEATHER     0x01    //Temperature and humidity of air

#ifdef MODULE
    #if MODULE == WEATHER
        #include "src/weather.h"
    #endif
#else
    #define MODULE  NOSENSOR
#endif


/*
 * GlobalVariable
 */
SimpleEspNowConnection SimpleEspConnection(SimpleEspNowRole::CLIENT);
static SensoriandoSensorDatum datum;

String ServerAddress;
int CounterSent=0;
int CounterError=0;
int Paired=0;


/*
 * Prototypes
 */
void espnow_init();
bool readConfig();
void OnSendError(uint8_t*);
void OnPaired(uint8_t *, String);
bool readConfig();
bool writeConfig();
void OnNewGatewayAddress(uint8_t *, String);
int readSensor(SensoriandoSensorDatum *);
void OnMessage(uint8_t*, const uint8_t*, size_t);
int RandomSensor(SensoriandoSensorDatum *);


/*
 * Main Program 
 */
void setup()
{
    Serial.begin(115200);
    pinMode(GPIO_PAIR, INPUT);

    // start SPIFFS file system. Ensure, sketch is uploaded with FS support !!!
    if ( ! SPIFFS.begin() ) {

#ifdef DEBUG
Serial.print("FS formating...");
#endif

        SPIFFS.format(); Serial.println("OK");
    }
  
    SimpleEspConnection.begin();
    SimpleEspConnection.setPairingBlinkPort(GPIO_LED);
    SimpleEspConnection.onMessage(&OnMessage);
    SimpleEspConnection.onPaired(&OnPaired);
    SimpleEspConnection.onNewGatewayAddress(&OnNewGatewayAddress);
    SimpleEspConnection.onSendError(&OnSendError);

#ifdef DEBUG
Serial.print("Client Address: ");Serial.println(WiFi.macAddress());
#endif

    espnow_init();

    #if MODULE == WEATHER
        if ( ! weather_init(&datum) ) {
            ESP.reset();
        }
    #else
        if ( ! RandomSensor(&datum) ) {
            ESP.reset();
        }
    #endif

#ifdef DEBUG
Serial.println("Waiting for sensor");
#endif

}

void loop()
{
    int i;
    int sensors;
    SimpleEspConnection.loop();
    static long updateelapsed=millis();

    if ( digitalRead(GPIO_PAIR) ) {

#ifdef DEBUG
Serial.println("Pairing started...");
#endif

        delay(DEBOUNCE);
        SimpleEspConnection.startPairing(120);
    }
  
    if ( Paired && (millis()-updateelapsed >= UPDATEELAPSED) ) {
        updateelapsed=millis();
        sensors = readSensor(&datum);

#ifdef DEBUG
Serial.printf("[%d] Sending data...\n", sensors);
#endif

        for (i=0; i<sensors; i++) {
#ifdef DEBUG
Serial.print("ptr ");Serial.println((int)&((&datum)[i]));
Serial.printf("id %d | value %f\n", (&datum)[i].id, (&datum)[i].value);
#endif

            if ( SimpleEspConnection.sendMessage((uint8_t *)&((&datum)[i]), sizeof(SensoriandoSensorDatum)) ) {
                CounterSent++;

#ifdef DEBUG
Serial.println("OK");
#endif
            } else {

#ifdef DEBUG
Serial.println("FAIL");
#endif
            }
        }

#ifdef DEBUG
Serial.print(" | Sent: ");Serial.print(CounterSent);
Serial.print(", error: ");Serial.print(CounterError);
Serial.println("");
#endif

    }
}


/*
 * functions 
 */
int RandomSensor(SensoriandoSensorDatum *datum)
{
    datum = (SensoriandoSensorDatum *)malloc(sizeof(SensoriandoSensorDatum));
    return datum != NULL;
}
 
void espnow_init() 
{
    if ( ! readConfig() ) {
        Serial.println("!!! [READ] Server address not save. Please pair first !!!");
        return;
    } else {
        Serial.print("Server address saved: ");Serial.println(ServerAddress);
    }

    if ( ! SimpleEspConnection.setServerMac(ServerAddress)) {// set the server address which is stored in EEPROM
        Serial.println("!!! [CONNECT] Server address not valid. Please pair again !!!");
        return;
    } else {
        Serial.print("Server address connected:");Serial.println(ServerAddress);
        Paired = 1;
    }
}

void OnSendError(uint8_t* ad)
{
#ifdef DEBUG    
Serial.println("Sending to " + SimpleEspConnection.macToStr(ad) + " was not possible!");
#endif

    CounterError++;
}

void OnPaired(uint8_t *ga, String ad)
{
#ifdef DEBUG    
Serial.println("EspNowConnection : Server '" + ad + " paired! ");
#endif

    SimpleEspConnection.endPairing();
    espnow_init();
}

bool readConfig()
{
    int i;

    if (!SPIFFS.exists("/setup.txt"))
        return false;

    File configFile = SPIFFS.open("/setup.txt", "r");
    if (!configFile)
        return false;

    for (i = 0; i < 12; i++) //Read server address
        ServerAddress += (char)configFile.read();

    configFile.close();
    return true;
}

bool writeConfig()
{
    File configFile = SPIFFS.open("/setup.txt", "w");
  
    if (!configFile)
        return false;

    configFile.print(ServerAddress.c_str());
    configFile.close();

    return true;
}

void OnNewGatewayAddress(uint8_t *ga, String ad)
{
    ServerAddress = ad;
    SimpleEspConnection.setServerMac(ga);
    Serial.println("Pairing mode finished...");
    writeConfig();
}

void OnConnected(uint8_t *ga, String ad)
{
    Serial.println(">> "); Serial.print(ad);
    Serial.println(">>[SERVER] connected!");
}

int readSensor(SensoriandoSensorDatum *datum) 
{
    int res=0;
 
    #if MODULE == WEATHER
        res=weather_read(datum);

#ifdef DEBUG
Serial.println(res);
#endif

    #else
        datum->stx = STX;
        datum->id = 0;
        datum->dt = NULL;
        datum->value = random(1,100)/PI;  
        datum->etx = ETX;
        
        res = 1;    
    #endif

#ifdef DEBUG
for (int i=0; i<res; i++) {
Serial.print("Bytes sent: ");Serial.println(sizeof(SensoriandoSensorDatum));
Serial.printf("stx=0x%02X, id=%d, value=%02f, etx=0x%02X\n", datum[i].stx, datum[i].id, datum[i].value, datum[i].etx);
}
#endif

    return res;
}

void OnMessage(uint8_t* ad, const uint8_t* message, size_t len)
{

#ifdef DEBUG    
Serial.print("Anything arrive from ");Serial.println((char *)ad);
Serial.println((char *)message);
#endif

}

 
