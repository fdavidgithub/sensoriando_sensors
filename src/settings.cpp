#include "settings.h"

void settings_close(File& file)
{
    file.close();
}

void settings_open(File& file)
{
    if ( !LittleFS.begin() ){
        LOGGER_SET("An Error has occurred while mounting LittleFS");
    } else {
        file = LittleFS.open(CONFIGFILE, "r");
    
        if( !file ) {
            LOGGER_SET("Failed to open file for reading");
        }
    }

}

char* settings_string(File& file, char* key)
{
    char line[50] = "";
    int lineIndex = 0;
    char c;
    char* value = NULL;
    char* string = NULL;

    while ( (lineIndex < sizeof(line)) && file.available() ) {            
        c = file.read();

        if ( c == '\n' )  {
            LOGGER_SET("Line: %s", line);

            if ( strstr(line, key) ) {
                value = strchr(line, '=') + 1;
                value = value + strspn(value, " \t\n\r\f\v");   //Move the number of leading whitespace
                LOGGER_SET("String value: %s\t width: %i", value, strlen(value));

                if ( value ) {
                    string = (char*)malloc( (strlen(value)+1) * sizeof(char));

                    if ( !string ) {
                        LOGGER_SET("Fatal erro, string set memory");
                        break;
                    }

                    strncpy(string, value, strlen(value));
                    string[strlen(value)] = '\0';  //Null-terminate the string 
                    break;
                }
            }

            lineIndex = 0;
        } else {
            if ( lineIndex < sizeof(line) - 1 ) {
                line[lineIndex] = c;
                lineIndex++;
            }
        }
    }

    if ( string ) {
        LOGGER_SET("Key %s: %s", key, string);
    } else {
        LOGGER_SET("%s param not found", key);
    }

    return string;
}


