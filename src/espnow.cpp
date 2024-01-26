#include "espnow.h"

bool espnow_writeconf(String serveraddress)
{
    File file;

    LOGGER_ESPNOW("Trying write server address in config file: %s", ESPNOWFILE);

    if ( !LittleFS.begin() ) {
        LOGGER_ESPNOW("An Error has occurred while mounting LittleFS");
        return false;
    } else {
        file = LittleFS.open(ESPNOWFILE, "w");
    
        if ( !file ) {
            LOGGER_ESPNOW("Failed to open file for writing");
            return false;
        }

        LOGGER_ESPNOW("Write AD: %s", serveraddress.c_str());
        file.print(serveraddress.c_str());
        file.close();
    }

    return true;
}

bool espnow_readconf(String *serveraddress)
{
    int i;
    File file;
    char cf;
    char sd[13];

    LOGGER_ESPNOW("Trying read server address in config file: %s", ESPNOWFILE);

    if ( !LittleFS.begin() ) {
        LOGGER_ESPNOW("An Error has occurred while mounting LittleFS");
        return false;
    } else {
        file = LittleFS.open(ESPNOWFILE, "r");
    
        if ( !file ) {
            LOGGER_ESPNOW("Failed to open file for reading");
            return false;
        }

        for (i = 0; i < 12; i++) {//Read server address
            cf = (char)file.read();
            sd[i] = cf;

            LOGGER_ESPNOW("%c_", cf);
        }

        sd[i] = '\0';
        String str((char *)sd);
        *serveraddress = String((char *)sd);

        file.close();
    }

    return true;
}

/*
void OnSendError(uint8_t* ad)
{
#ifdef DEBUG_HUB    
Serial.println("Sending to " + ConnectEsp.macToStr(ad) + " was not possible!");
#endif
}

void OnPaired(uint8_t *ga, String ad)
{
#ifdef DEBUG_HUB    
Serial.println("HUB Connection : Server '" + ad + " paired! ");
#endif

    ConnectEsp.endPairing();

    delay(1000);
    ESP.reset();
}

void OnNewGatewayAddress(uint8_t *ga, String ad)
{
    ConnectEsp.setServerMac(ga);

#ifdef DEBUG_HUB
Serial.println("Pairing mode finished...");
#endif

    writeConfig((char *)ad.c_str());
}

void OnConnected(uint8_t *ga, String ad)
{
#ifdef DEBUG_HUB
    Serial.println(">> "); Serial.print(ad);
    Serial.println(">>[SERVER] connected!");
#endif
}

void OnMessage(uint8_t* ad, const uint8_t* message, size_t len)
{

#ifdef DEBUG_HUB    
Serial.print("Anything arrive from ");Serial.println((char *)ad);
Serial.println((char *)message);
#endif

}
*/

byte espnow_connected(esp_connection *conn) 
{
    return 1;//espnow->paired;    
}

void espnow_reset()
{
    LOGGER_ESPNOW("Trying erase server address in config file: %s", ESPNOWFILE);

    if ( !LittleFS.begin() ) {
        LOGGER_ESPNOW("An Error has occurred while mounting LittleFS");
    } else {
        if ( LittleFS.exists(ESPNOWFILE) ) {
            if ( LittleFS.remove(ESPNOWFILE) ) {
                LOGGER_ESPNOW("Config file deleted");
            } else { 
                LOGGER_ESPNOW("Error while delete config file");
            }  
        } else {    
            LOGGER_ESPNOW("Config file do not exists");
        }
    }    
} 

byte espnow_init(esp_connection *conn, \
                    SendErrorFunction se, SendDoneFunction sd, \
                    NewGatewayAddressFunction nga, \
                    PairingFinishedFunction pf) 
{
    byte res=0;
    String serveraddress;


    //Setting ESPNOW
    conn->begin();
    conn->setPairingBlinkPort(GPIO_CONFIG);
//    conn->onMessage(m);
//    conn->onPaired(p);
    conn->onPairingFinished(pf);
    conn->onNewGatewayAddress(nga);
    conn->onSendError(se);
    conn->onSendDone(sd);

    if ( ! espnow_readconf(&serveraddress) ) {
        LOGGER_ESPNOW("Server address not saved");
        return res;
    } else {
        LOGGER_ESPNOW("Server address saved");
    }

    if ( ! conn->setServerMac(serveraddress) ) {// set the server address which is stored in EEPROM
        LOGGER_ESPNOW("Connect NOT VALID. Please pair again !!!");
    } else {
        LOGGER_ESPNOW("Server address connected: ");
        //LOGGER_ESPNOW(serveraddress);
        res=1;
    }

    return res;
}

byte espnow_pair(esp_connection *conn)
{
    return conn->startPairing(PAIRTIME);
}

byte espnow_send(esp_connection *conn, SensoriandoSensorDatum *datum)
{
    return conn->sendMessage((uint8_t *)datum, sizeof(SensoriandoSensorDatum)); 
}

