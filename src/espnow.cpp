#include "espnow.h"

bool espnow_writeconf(String serveraddress)
{
    File configfile;

    configfile = SPIFFS.open(CONFIGFILE, "w");
    
    if (!configfile)
        return false;

#ifdef DEBUG_ESPNOW
Serial.print("[DEBUG_ESPNOW] Write AD: ");Serial.println(serveraddress.c_str());
#endif

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

#ifdef DEBUG_ESPNOW
Serial.print("[DEBUG_ESPNOW] Trying read server address in config file: ");
Serial.println(CONFIGFILE);
#endif

    // start SPIFFS file system. Ensure, sketch is uploaded with FS support !!!
    if ( ! SPIFFS.begin() ) {
        espnow_reset();
    }

    if (!SPIFFS.exists(CONFIGFILE)) {
#ifdef DEBUG_ESPNOW
Serial.println("[DEBUG_ESPNOW] Config file not found");
#endif
        return false;
    }

    configfile = SPIFFS.open(CONFIGFILE, "r");
    
    if (!configfile) {
#ifdef DEBUG_ESPNOW
Serial.println("[DEBUG_ESPNOW] Do not can open config file");
#endif
        return false;
    }

    for (i = 0; i < 12; i++) {//Read server address
        cf = (char)configfile.read();
        sd[i] = cf;

#ifdef DEBUG_ESPNOW
Serial.print(cf);Serial.print("_");
#endif
    }

    sd[i] = '\0';
    String str((char *)sd);
    *serveraddress = String((char *)sd);

#ifdef DEBUG_ESPNOW
Serial.println(str);
#endif

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

#ifdef DEBUG_ESPNOW
Serial.print("[DEBUG_ESPNOW] FS formating...");
#endif

    SPIFFS.format();

#ifdef DEBUG_ESPNOW
Serial.println("OK");
#endif
} 

byte espnow_init(espnow_connection *conn, MessageFunction m, \
                    SendErrorFunction se, SendDoneFunction sd, \
                    NewGatewayAddressFunction nga, ConnectedFunction c, \
                    PairedFunction p, PairingFinishedFunction pf) 
{
    byte res=0;
    String serveraddress;


    //Setting ESPNow
    conn->begin();
    conn->setPairingBlinkPort(GPIO_LED);
    conn->onMessage(m);
    conn->onPaired(p);
    conn->onPairingFinished(pf);
    conn->onNewGatewayAddress(nga);
    conn->onSendError(se);
    conn->onSendDone(sd);

    if ( ! espnow_readconf(&serveraddress) ) {
#ifdef DEBUG_ESPNOW      
Serial.println("[DEBUG_ESPNOW] Server address not saved");
#endif
      return res;
    } else {
#ifdef DEBUG_ESPNOW
Serial.println("[DEBUG_ESPNOW] Server address saved");
#endif
    }

    if ( ! conn->setServerMac(serveraddress) ) {// set the server address which is stored in EEPROM

#ifdef DEBUG_ESPNOW
Serial.println("!!! [DEBUG_ESPNOW] Connect NOT VALID. Please pair again !!!");
#endif

    } else {

#ifdef DEBUG_ESPNOW
Serial.print("[DEBUG_ESPNOW] Server address connected: ");
Serial.println(serveraddress);
#endif

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

