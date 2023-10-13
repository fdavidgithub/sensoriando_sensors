#include "espnow.h"

bool espnow_writeconf(String serveraddress)
{
    File configfile;

    configfile = SPIFFS.open(CONFIGFILE, "w");
    
    if (!configfile)
        return false;
        
    LOGGER_ESP("Write AD: %s", serveraddress.c_str());

    configfile.print(serveraddress.c_str());
    configfile.close();

    return true;
}

bool espnow_readconf(String *serveraddress)
{
    int i;
    File configfile;
    char cf;
    char sd[13];

    LOGGER_ESP("Trying read server address in config file: %s", CONFIGFILE);

    // start SPIFFS file system. Ensure, sketch is uploaded with FS support !!!
    if ( ! SPIFFS.begin() ) {
        SPIFFS.format();
    }

    if ( ! SPIFFS.exists(CONFIGFILE)) {
        LOGGER_ESP("Config file not found");
        return false;
    }

    configfile = SPIFFS.open(CONFIGFILE, "r");
    
    if (!configfile) {
        LOGGER_ESP("Do not can open config file");
        return false;
    }

    for (i = 0; i < 12; i++) {//Read server address
        cf = (char)configfile.read();
        sd[i] = cf;

        LOGGER_ESP("%c_", cf);
    }

    sd[i] = '\0';
    String str((char *)sd);
    *serveraddress = String((char *)sd);

    configfile.close();
    return true;
}

/*
void OnSendError(uint8_t* ad)
{
#ifdef DEBUG_ESPNOW    
Serial.println("Sending to " + ConnectEsp.macToStr(ad) + " was not possible!");
#endif
}

void OnPaired(uint8_t *ga, String ad)
{
#ifdef DEBUG_ESPNOW    
Serial.println("EspNowConnection : Server '" + ad + " paired! ");
#endif

    ConnectEsp.endPairing();

    delay(1000);
    ESP.reset();
}

void OnNewGatewayAddress(uint8_t *ga, String ad)
{
    ConnectEsp.setServerMac(ga);

#ifdef DEBUG_ESPNOW
Serial.println("Pairing mode finished...");
#endif

    writeConfig((char *)ad.c_str());
}

void OnConnected(uint8_t *ga, String ad)
{
#ifdef DEBUG_ESPNOW
    Serial.println(">> "); Serial.print(ad);
    Serial.println(">>[SERVER] connected!");
#endif
}

void OnMessage(uint8_t* ad, const uint8_t* message, size_t len)
{

#ifdef DEBUG_ESPNOW    
Serial.print("Anything arrive from ");Serial.println((char *)ad);
Serial.println((char *)message);
#endif

}
*/

byte espnow_connected(espnow_connection *conn) 
{
    return 1;//espnow->paired;    
}

void espnow_reset()
{
    LOGGER_ESP("Deleting config file...");
    SPIFFS.remove(CONFIGFILE);
    LOGGER_ESP("OK");
} 

byte espnow_init(espnow_connection *conn, \
                    SendErrorFunction se, SendDoneFunction sd, \
                    NewGatewayAddressFunction nga, \
                    PairingFinishedFunction pf) 
{
    byte res=0;
    String serveraddress;


    //Setting ESPNow
    conn->begin();
    conn->setPairingBlinkPort(GPIO_CONFIG);
//    conn->onMessage(m);
//    conn->onPaired(p);
    conn->onPairingFinished(pf);
    conn->onNewGatewayAddress(nga);
    conn->onSendError(se);
    conn->onSendDone(sd);

    if ( ! espnow_readconf(&serveraddress) ) {
        LOGGER_ESP("Server address not saved");
        return res;
    } else {
        LOGGER_ESP("Server address saved");
    }

    if ( ! conn->setServerMac(serveraddress) ) {// set the server address which is stored in EEPROM
        LOGGER_ESP("Connect NOT VALID. Please pair again !!!");
    } else {
        LOGGER_ESP("Server address connected: ");
        //LOGGER_ESP(serveraddress);
        res=1;
    }

    return res;
}

byte espnow_pair(espnow_connection *conn)
{
    return conn->startPairing(PAIRTIME);
}

byte espnow_send(espnow_connection *conn, SensoriandoSensorDatum *datum)
{
    return conn->sendMessage((uint8_t *)datum, sizeof(SensoriandoSensorDatum)); 
}

