


#include "../pa_touchScreen.h"

/****************************************************
 * driver 层代码
 * ***************************************************/

#ifdef ESP32
#define pa_touchScreen_IRQ 15
#define pa_touchScreen_CS 2
#define pa_touchScreen_MOSI 4
#define pa_touchScreen_MISO 27
#define pa_touchScreen_CLK 26

void pa_touchScreen::Hardware_Init() 
{
    pinMode(pa_touchScreen_IRQ,INPUT);
    pinMode(pa_touchScreen_CS,OUTPUT);
    pinMode(pa_touchScreen_MOSI,OUTPUT);
    pinMode(pa_touchScreen_MISO,INPUT);
    pinMode(pa_touchScreen_CLK,OUTPUT);
}

uint8_t pa_touchScreen::Hardware_ReadIRQ() 
{
    return digitalRead(pa_touchScreen_IRQ);
}

void pa_touchScreen::Hardware_SetCS(uint8_t state) 
{
    digitalWrite(pa_touchScreen_CS,state);
}

void pa_touchScreen::Hardware_setMOSI(uint8_t state) 
{
    digitalWrite(pa_touchScreen_MOSI,state);
}

uint8_t pa_touchScreen::Hardware_ReadMISO() 
{
    return digitalRead(pa_touchScreen_MISO);
}

void pa_touchScreen::Hardware_setCLK(uint8_t state) 
{
    digitalWrite(pa_touchScreen_CLK,state);
}

#endif