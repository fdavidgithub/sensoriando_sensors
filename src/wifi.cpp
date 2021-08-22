#include "wifi.h"

void wifi_reset(wifi_connection *conn)
{
    conn->resetSettings();
} 

byte wifi_connected()
{
    return WiFi.status() == WL_CONNECTED;
}

byte wifi_init(wifi_connection *conn, uint8_t *mac)
{
    char namedevice[30] = PREFIXAPMODE;


    /*
     * Start AP mode 
     */
    WiFi.macAddress(mac);
    conn->setTimeout(MODEAP_TIMEOUT);

    for (int i=0; i<sizeof(mac); i++) {
        sprintf(namedevice, "%s%02X", namedevice, mac[i]);
    }

    if( ! conn->autoConnect(namedevice) ) {

#ifdef DEBUG_WIFI
Serial.println("[DEBUG_WIFI] Failed to connect or hit timeout");
#endif

//    WiFi.forceSleepBegin();
    } 

    return wifi_connected();
}

byte wifi_send(SensoriandoObj *obj, SensoriandoSensorDatum *datum)
{
    SensoriandoParser parser;

    strcpy(parser.uuid, datum->uuid);
    parser.value = datum->value;
    parser.dt = datum->dt;
    parser.id = datum->id;
    
    return sensoriandoSendValue(obj, &parser);
}

