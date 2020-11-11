

#ifndef __PA_OLED_DRV_H__
#define __PA_OLED_DRV_H__
void OLED_initSpiGpio();
void OLED_setCS(char state);
void OLED_setDC(char state);
void OLED_setRST(char state);
#endif // __PA_OLED_DRV_H__