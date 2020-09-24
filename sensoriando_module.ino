/*
 *
 */
#include <sensoriando.h>
#include <SimpleEspNowConnection.h>
#include "src/gpio.h"
#include "src/messages.h"


/*
 * MACROS
 */
#define DEBUG

#define NOSENSOR    0x00    //Random values
#define WEATHER     0x01    //Temperature, humidity and pressue of air

#define MODULE      NOSENSOR


/*
 * GlobalVariable
 */
SimpleEspNowConnection SimpleEspConnection(SimpleEspNowRole::CLIENT);
SensoriandoSensorDatum myData;

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
void readSensor();
void OnMessage(uint8_t*, const uint8_t*, size_t);


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
}

void loop()
{
    SimpleEspConnection.loop();

    if ( digitalRead(GPIO_PAIR) ) {

#ifdef DEBUG
Serial.println("Pairing started...");
#endif

        delay(1000);
        SimpleEspConnection.startPairing(120);
    }
  
    if ( Paired ) {

#ifdef DEBUG
Serial.println("Sending datum...");
#endif

        readSensor();
    
        if ( SimpleEspConnection.sendMessage((uint8_t *)&myData, sizeof(myData)) ) {
            CounterSent++;

#ifdef DEBUG
Serial.println("OK");
#endif

        } else {

#ifdef DEBUG
Serial.println("FAIL");
#endif
        }

#ifdef DEBUG
Serial.print(" | Sent: ");Serial.print(CounterSent);
Serial.print(", error: ");Serial.print(CounterError);
Serial.println("");
#endif

    }

    delay(1000);
}


/*
 * functions 
 */
void espnow_init() 
{
  if (!readConfig())
  {
    Serial.println("!!! [READ] Server address not save. Please pair first !!!");
    return;
  } else {
    Serial.print("Server address saved: ");Serial.println(ServerAddress);
  }

  if (!SimpleEspConnection.setServerMac(ServerAddress)) // set the server address which is stored in EEPROM
  {
    Serial.println("!!! [CONNECT] Server address not valid. Please pair again !!!");
    return;
  } else {
    Serial.print("Server address connected:");Serial.println(ServerAddress);
    Paired = 1;
  }
}

void OnSendError(uint8_t* ad)
{
  Serial.println("Sending to " + SimpleEspConnection.macToStr(ad) + " was not possible!");
  CounterError++;
}

void OnPaired(uint8_t *ga, String ad)
{
  Serial.println("EspNowConnection : Server '" + ad + " paired! ");
  SimpleEspConnection.endPairing();

  espnow_init();
}

bool readConfig()
{
  if (!SPIFFS.exists("/setup.txt"))
    return false;

  File configFile = SPIFFS.open("/setup.txt", "r");
  if (!configFile)
    return false;

  for (int i = 0; i < 12; i++) //Read server address
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

void readSensor() 
{
  myData.stx = 0x02;
  myData.id = 42;
  myData.dt = NULL;
  myData.value = random(1,100)/PI;  
  myData.etx = 0x03;

  Serial.print("Bytes sent: ");Serial.println(sizeof(myData));
  Serial.printf("stx=0x%02X, id=%d, value=%02f, etx=0x%02X\n", myData.stx, myData.id, myData.value, myData.etx);
}

void OnMessage(uint8_t* ad, const uint8_t* message, size_t len)
{
    Serial.print("Anything arrive from ");Serial.println((char *)ad);
    Serial.println((char *)message);
}

 
