#include "wifi.h"

void wifi_reset(wifi_connection *conn)
{
    conn->resetSettings();
} 

byte wifi_connected()
{
    return WiFi.status() == WL_CONNECTED;
}

byte wifi_init(wifi_connection *conn)
{
    char namedevice[30] = PREFIXAPMODE;
    byte mac[6];
    int i;

    /*
     * Start AP mode 
     */
    WiFi.macAddress(mac);

    for (i=0; i<sizeof(mac); i++) {
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

