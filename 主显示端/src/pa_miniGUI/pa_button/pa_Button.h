#ifndef __PA_BUTTON_H__
#define __PA_BUTTON_H__

#include "Arduino.h"
class pa_Button
{
public:
    pa_Button();
    void init(int x,int y,int w,int h);
    void loop();
    void (*buttonCallback)();
    static bool isPressed();
    static void setPos(int x,int y);
    
private:
    static int Px;
    static int Py;
    int x;
    int y;
    int w;
    int h;
    bool lastDown=false;
    bool down;
    int downMillis;
};
#endif // __PA_BUTTON_H__