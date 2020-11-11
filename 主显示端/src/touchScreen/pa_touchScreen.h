
#ifndef __PA_TOUCHSCREEN_H__
#define __PA_TOUCHSCREEN_H__

extern "C"
{
}
#include "Arduino.h"
class pa_touchScreen
{

public:
  static pa_touchScreen instance;
  pa_touchScreen();
  void init(
      int screenW,
      int screenH,
      int xBeginRaw,
      int xEndRaw,
      int yBeginRaw,
      int yEndRaw,
      int sampleTime);
  uint8_t readRaw(uint16_t Coordinates[2]);
  void turnRawToScreen(uint16_t Coordinates[2]);
  uint8_t isPressed();

  void Hardware_SetCS(uint8_t state);

private:
  struct ConfigModel
  {
    int screenW;
    int screenH;
    int SampleCount;
    int xBeginRaw;
    int xEndRaw;
    int yBeginRaw;
    int yEndRaw;
  };
  //不要修改此处的数据。因为只是个默认值
  ConfigModel config = {
      0,
      0,
      2, //SampleCount,
      0,
      0,
      0,
      0};
  enum CMDs
  {
    CMD_RDY = 0X90,
    CMD_RDX = 0XD0
  };

  void Hardware_Init();
  uint8_t Hardware_ReadIRQ();
  void Hardware_setMOSI(uint8_t state);
  uint8_t Hardware_ReadMISO();
  void Hardware_setCLK(uint8_t state);

  uint16_t spiRead();
  void spiWrite(uint8_t value);
};

#endif