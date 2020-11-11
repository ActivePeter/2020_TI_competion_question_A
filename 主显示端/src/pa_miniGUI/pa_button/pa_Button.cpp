#include "pa_Button.h"
pa_Button::pa_Button() {}
int pa_Button::Px;
int pa_Button::Py;

void pa_Button::init(int x1, int y1, int w1, int h1)
{
    x = x1;
    y = y1;
    w = w1;
    h = h1;
}

void pa_Button::loop()
{
    down = pa_Button::isPressed();
    // Serial.printf("down:%d", down);
    if (lastDown == false && down == true)
    {
        downMillis = millis();
    }
    // Serial.printf("%d %d\r\n",pa_Button::Px,pa_Button::Py);
    // if(down&&pa_Button::Px > x && pa_Button::Px < x + w && pa_Button::Py > y && pa_Button::Py < y + h){
    //     Serial.print("down");
    //     buttonCallback();
    // }
    if (lastDown == true && down == false)
    {
        if (millis() - downMillis > 80)
        {
            // Serial.printf("%d %d\r\n", pa_Button::Px, pa_Button::Py);
            // Serial.printf("press valid\r\n");
            if (pa_Button::Px > x && pa_Button::Px < x + w && pa_Button::Py > y && pa_Button::Py < y + h)
            {

                buttonCallback();
            }
        }
    }
    lastDown = down;
}
void pa_Button::setPos(int x1, int y1)
{
    if (pa_Button::isPressed())
    {
        pa_Button::Px = x1;
        pa_Button::Py = y1;
    }
}