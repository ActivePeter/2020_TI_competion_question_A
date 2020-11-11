

/////////////////////////////////////////////////////////////////////////////////
//              GND   电源地
//              VCC   接5V或3.3v电源
//              SCL   接PA5（SCL）
//              SDA   接PA7（SDA）
/////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "pa_oled.h"
#include "stdlib.h"
#include "pa_oledfont.h"
#include "pa_HardwareIIC/pa_HardwareIIC.h"
#include "pa_oled_drv.h"
}
pa_IICSettingStruct OLED_IICSettingStruct;
/****************************************************/
////////////////////////////////////////////////////////////////////////////////
void OLED_Write_IIC_Command(unsigned char IIC_Command)
{
    pa_IIC_writeLen(SSD1306_I2C_ADDRESS, 0x00, 1, &IIC_Command, OLED_IICSettingStruct);
}
/**********************************************
// IIC Write Data
**********************************************/
void OLED_Write_IIC_Data(unsigned char IIC_Data)
{
    pa_IIC_writeLen(SSD1306_I2C_ADDRESS, 0x40, 1, &IIC_Data, OLED_IICSettingStruct);
}
void OLED_WR_Byte(unsigned char dat, unsigned char cmd)
{

    if (cmd)
    {
        OLED_Write_IIC_Data(dat);
    }
    else
    {
        OLED_Write_IIC_Command(dat);
    }
}

/********************************************
// fill_Picture
********************************************/
void fill_picture(unsigned char *fill_Data)
{
    OLED_WR_Byte(0x21, 0); //page0-page1
    OLED_WR_Byte(0x00, 0); //low column start address
    OLED_WR_Byte(0x7f, 0); //high column start a
    OLED_WR_Byte(0x22, 0); //page0-page1
    OLED_WR_Byte(0x00, 0); //low column start address
    OLED_WR_Byte(0x07, 0); //high column start a
    // pa_IIC_writeLen(SSD1306_I2C_ADDRESS, 0x40,128,fill_Data);
    // pa_IIC_writeLen(SSD1306_I2C_ADDRESS, 0x40,128,fill_Data+);
    // pa_IIC_writeLen(SSD1306_I2C_ADDRESS, 0x40,128,fill_Data);
    // unsigned char m, n;
    for (char m = 0; m < 8; m++)
    {
        // OLED_WR_Byte(0xb0 + m, 0); //page0-page1
        // OLED_WR_Byte(0x00, 0);     //low column start address
        // OLED_WR_Byte(0x10, 0);     //high column start address
        pa_IIC_writeLen(SSD1306_I2C_ADDRESS, 0x40, 128, fill_Data + 128 * m, OLED_IICSettingStruct);
        // for (n = 0; n < 128; n++)
        // {
        //     OLED_WR_Byte(fill_Data, 1);
        // }
    }
}

/***********************Delay****************************************/
void Delay_50ms(unsigned int Del_50ms)
{
    unsigned int m;
    for (; Del_50ms > 0; Del_50ms--)
        for (m = 6245; m > 0; m--)
            ;
}

void Delay_1ms(unsigned int Del_1ms)
{
    unsigned char j;
    while (Del_1ms--)
    {
        for (j = 0; j < 123; j++)
            ;
    }
}

//坐标设置

void OLED_Set_Pos(unsigned char x, unsigned char y)
{
    OLED_WR_Byte(0xb0 + y, OLED_CMD);
    OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
    OLED_WR_Byte((x & 0x0f), OLED_CMD);
}
//开启OLED显示
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD); //SET DCDC命令
    OLED_WR_Byte(0X14, OLED_CMD); //DCDC ON
    OLED_WR_Byte(0XAF, OLED_CMD); //DISPLAY ON
}
//关闭OLED显示
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD); //SET DCDC命令
    OLED_WR_Byte(0X10, OLED_CMD); //DCDC OFF
    OLED_WR_Byte(0XAE, OLED_CMD); //DISPLAY OFF
}
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
void OLED_Clear(void)
{
    unsigned char i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD); //设置页地址（0~7）
        OLED_WR_Byte(0x00, OLED_CMD);     //设置显示位置—列低地址
        OLED_WR_Byte(0x10, OLED_CMD);     //设置显示位置—列高地址
        for (n = 0; n < 128; n++)
            OLED_WR_Byte(0, OLED_DATA);
    } //更新显示
}
void OLED_On(void)
{
    unsigned char i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD); //设置页地址（0~7）
        OLED_WR_Byte(0x00, OLED_CMD);     //设置显示位置—列低地址
        OLED_WR_Byte(0x10, OLED_CMD);     //设置显示位置—列高地址
        for (n = 0; n < 128; n++)
            OLED_WR_Byte(1, OLED_DATA);
    } //更新显示
}
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示
//size:选择字体 16/12
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr, unsigned char Char_Size)
{
    unsigned char c = 0, i = 0;
    c = chr - ' '; //得到偏移后的值
    if (x > Max_Column - 1)
    {
        x = 0;
        y = y + 2;
    }
    if (Char_Size == 16)
    {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
    }
    else
    {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WR_Byte(F6x8[c][i], OLED_DATA);
    }
}
//m^n函数
unsigned int oled_pow(unsigned char m, unsigned char n)
{
    unsigned int result = 1;
    while (n--)
        result *= m;
    return result;
}
//显示2个数字
//x,y :起点坐标
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);
void OLED_ShowNum(unsigned char x, unsigned char y, unsigned int num, unsigned char len, unsigned char size2)
{
    unsigned char t, temp;
    unsigned char enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                OLED_ShowChar(x + (size2 / 2) * t, y, ' ', size2);
                continue;
            }
            else
                enshow = 1;
        }
        OLED_ShowChar(x + (size2 / 2) * t, y, temp + '0', size2);
    }
}
//显示一个字符号串,suze:12/16
void OLED_ShowString(unsigned char x, unsigned char y, char *chr, unsigned char Char_Size)
{
    unsigned char j = 0;
    while (chr[j] != '\0')
    {
        OLED_ShowChar(x, y, chr[j], Char_Size);
        x += 8;
        if (x > 120)
        {
            x = 0;
            y += 2;
        }
        j++;
    }
}
//显示汉字
void OLED_ShowCHinese(unsigned char x, unsigned char y, unsigned char no)
{
    unsigned char t, adder = 0;
    OLED_Set_Pos(x, y);
    for (t = 0; t < 16; t++)
    {
        OLED_WR_Byte(Hzk[2 * no][t], OLED_DATA);
        adder += 1;
    }
    OLED_Set_Pos(x, y + 1);
    for (t = 0; t < 16; t++)
    {
        OLED_WR_Byte(Hzk[2 * no + 1][t], OLED_DATA);
        adder += 1;
    }
}
/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
    unsigned int j = 0;
    unsigned char x, y;

    if (y1 % 8 == 0)
        y = y1 / 8;
    else
        y = y1 / 8 + 1;
    for (y = y0; y < y1; y++)
    {
        OLED_Set_Pos(x0, y);
        for (x = x0; x < x1; x++)
        {
            OLED_WR_Byte(BMP[j++], OLED_DATA);
        }
    }
}

//初始化SSD1306
void OLED_Init()
{
    OLED_IICSettingStruct.delay = 60;

    OLED_WR_Byte(0xAE, OLED_CMD); //--display off
    OLED_WR_Byte(0x20, OLED_CMD);
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0x00, OLED_CMD); //---set low column address
    OLED_WR_Byte(0x10, OLED_CMD); //---set high column address
    OLED_WR_Byte(0x40, OLED_CMD); //--set start line address
    OLED_WR_Byte(0xB0, OLED_CMD); //--set page address
    OLED_WR_Byte(0x81, OLED_CMD); // contract control
    OLED_WR_Byte(0xFF, OLED_CMD); //--128
    OLED_WR_Byte(0xA1, OLED_CMD); //set segment remap
    OLED_WR_Byte(0xA6, OLED_CMD); //--normal / reverse
    OLED_WR_Byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3F, OLED_CMD); //--1/32 duty
    OLED_WR_Byte(0xC8, OLED_CMD); //Com scan direction
    OLED_WR_Byte(0xD3, OLED_CMD); //-set display offset
    OLED_WR_Byte(0x00, OLED_CMD); //

    OLED_WR_Byte(0xD5, OLED_CMD); //set osc division
    OLED_WR_Byte(0x80, OLED_CMD); //

    OLED_WR_Byte(0xD8, OLED_CMD); //set area color mode off
    OLED_WR_Byte(0x05, OLED_CMD); //

    OLED_WR_Byte(0xD9, OLED_CMD); //Set Pre-Charge Period
    OLED_WR_Byte(0xF1, OLED_CMD); //

    OLED_WR_Byte(0xDA, OLED_CMD); //set com pin configuartion
    OLED_WR_Byte(0x12, OLED_CMD); //

    OLED_WR_Byte(0xDB, OLED_CMD); //set Vcomh
    OLED_WR_Byte(0x30, OLED_CMD); //

    OLED_WR_Byte(0x8D, OLED_CMD); //set charge pump enable
    OLED_WR_Byte(0x14, OLED_CMD); //

    OLED_WR_Byte(0xAF, OLED_CMD); //--turn on oled panel

    // oled_write_cmd(0x20);    // Set Memory Addressing Mode (20h)
    // oled_write_cmd(0x02);
}
