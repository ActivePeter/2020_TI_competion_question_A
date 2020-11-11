
#include "Arduino.h"
#include "../Ads_112c04.h"

#ifdef ESP32
void Ads_112c04::initHardware()
{
    pinMode(32,INPUT);
}

char Ads_112c04::getDrdyState()
{
    return digitalRead(32);
}
#endif