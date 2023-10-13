#include "log.h"

void logger(char *source, char *string)
{
    Serial.printf("[%s] %s\n", source, string);
}

void logger(char *source, int value)
{
    Serial.printf("[%s] %i\n", source, value);
}

void logger(char *source, double value)
{
    Serial.printf("[%s] %f\n", source, value);
}

void logargs(char *source, char *string, ...)
{
    char buffer[256];
    va_list args;
    
    va_start(args, string);
    vsnprintf(buffer, sizeof(buffer), string, args);
    va_end(args);

    Serial.printf("[%s] ", source);
    Serial.println(buffer);
}
