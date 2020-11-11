
extern "C"{
    #include "../pa_oled_drv.h"
}

#ifdef ESP32

// D0                         4 线 ISP 接口模式：时钟线（CLK）
// D1                         4 线 ISP 接口模式：串行数据线（MOSI）
void OLED_initSpiGpio(){
}

void OLED_setCS(char state){
}

void OLED_setDC(char state){
}

void OLED_setRST(char state){
}
#endif
