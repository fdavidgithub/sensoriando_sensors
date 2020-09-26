#include <SimpleDHT.h>
#include "gpio.h"

/* 
 * GlobalVariables
 */
byte ValueTemperature, ValueHumidity;
SimpleDHT11 dht(GPIO_WEATHER);

void setup()
{
    Serial.begin(115200);       
}

void loop()
{
  int res = dht.read(&ValueTemperature, &ValueHumidity, NULL);
  static int count=0;
  
switch (res) {
  case SimpleDHTErrSuccess: break;
  case SimpleDHTErrStartLow: Serial.println("Error to wait for start low signal.");break;
  case SimpleDHTErrStartHigh: Serial.println("Error to wait for start high signal.");break;
  case SimpleDHTErrDataLow: Serial.println("Error to wait for data start low signal.");break;
  case SimpleDHTErrDataRead: Serial.println("Error to wait for data read signal.");break;
  case SimpleDHTErrDataEOF: Serial.println("Error to wait for data EOF signal.");break;
  case SimpleDHTErrDataChecksum: Serial.println("Error to validate the checksum.");break;
  case SimpleDHTErrZeroSamples: Serial.println("Error when temperature and humidity are zero, it shouldn't happen.");break;
  case SimpleDHTErrNoPin: Serial.println(" Error when pin is not initialized.");break;    
  default: Serial.print("Error code: ");Serial.println(res);break;
}

Serial.print(count);Serial.print(". ");
Serial.print(ValueTemperature); Serial.print("C, "); 
Serial.print(ValueHumidity); Serial.println("%");
  
  delay(1000);      
  count++;   
}
