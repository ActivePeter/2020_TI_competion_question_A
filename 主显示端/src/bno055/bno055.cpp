extern "C"
{
#include "bno055.h"
#include "pa_HardwareIIC/pa_HardwareIIC.h"
}
#include "Arduino.h"
#define pa_BNO055_chosenAddress (0x29)

unsigned long timeStart = 0;
unsigned long timeStop = 0;
unsigned long timeinterval = 0;
unsigned char startflag = 0;
extern float bno055_InitZ;

void pa_BNO055_writeIIC(uint8_t add, uint8_t reg, uint8_t data)
{
  pa_IICSettingStruct a;
  pa_IIC_writeLen(add, reg, 1, &data, a);
}
uint8_t pa_BNO055_readIIC(uint8_t add, uint8_t reg)
{
  pa_IICSettingStruct a;
  uint8_t data[1];
  pa_IIC_readLen(add, reg, 1, data, a);
  return data[0];
}
void pa_BNO055_init()
{

  pa_BNO055_writeIIC(pa_BNO055_chosenAddress, BNO055_SYS_TRIGGER_ADDR, 0x20);
  pa_BNO055_writeIIC(pa_BNO055_chosenAddress, BNO055_SYS_TRIGGER_ADDR, 0x20);
  delay(700);
  // pa_IIC_init();
  unsigned char id = pa_BNO055_readIIC(pa_BNO055_chosenAddress, BNO055_CHIP_ID_ADDR); //bno055_readData(BNO055_CHIP_ID, &id, 1);//ok
                                                                                      //    if (id != BNO055_ID) {
                                                                                      //    delay(1000); // hold on for boot
                                                                                      //    id = pa_IIC_read8(pa_BNO055_chosenAddress,BNO055_CHIP_ID_ADDR);}
  if (id != BNO055_ID)
  {
    while (1)
    {
    }
    // pa_gotoErrorBlink();
    return; // still not? ok bail
  }

  pa_BNO055_writeIIC(pa_BNO055_chosenAddress, BNO055_PAGE_ID_ADDR, 0);        //bno055_setPage
  pa_BNO055_writeIIC(pa_BNO055_chosenAddress, BNO055_SYS_TRIGGER_ADDR, 0x00); //bno055_writeData(BNO055_SYS_TRIGGER, 0x0);//ok

  // Select BNO055 config mode
  pa_BNO055_writeIIC(pa_BNO055_chosenAddress, BNO055_OPR_MODE_ADDR, OPERATION_MODE_CONFIG); //bno055_setOperationModeConfig();//ok
  delay(19);
  delay(10);

  pa_BNO055_writeIIC(pa_BNO055_chosenAddress, BNO055_OPR_MODE_ADDR, OPERATION_MODE_NDOF); //bno055_setOperationModeNDOF();
  delay(7);
}

bno055_vector_t pa_BNO055_getVector()
{
  pa_BNO055_writeIIC(pa_BNO055_chosenAddress, BNO055_PAGE_ID_ADDR, 0); //bno055_setPage
  unsigned char buffer[8];                                             // Quaternion need 8 bytes

  // if (BNO055_EULER_H_LSB_ADDR == BNO055_QUATERNION_DATA_W_LSB_ADDR)
  //   pa_IIC_read8(pa_BNO055_chosenAddress, BNO055_EULER_H_LSB_ADDR);
  // else
  {
    pa_IICSettingStruct a;
    pa_IIC_readLen(pa_BNO055_chosenAddress, BNO055_EULER_H_LSB_ADDR, sizeof(buffer), buffer, a);
  }

  double scale = 1;

  if (BNO055_EULER_H_LSB_ADDR == BNO055_MAG_DATA_X_LSB_ADDR)
  {
    scale = 16;
  }
  else if (BNO055_EULER_H_LSB_ADDR == BNO055_ACCEL_DATA_X_LSB_ADDR || BNO055_EULER_H_LSB_ADDR == BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR || BNO055_EULER_H_LSB_ADDR == BNO055_GRAVITY_DATA_X_LSB_ADDR)
  {
    scale = 100;
  }
  else if (BNO055_EULER_H_LSB_ADDR == BNO055_GYRO_DATA_X_LSB_ADDR)
  {
    scale = 16;
  }
  else if (BNO055_EULER_H_LSB_ADDR == BNO055_EULER_H_LSB_ADDR)
  {
    scale = 16;
  }
  else if (BNO055_EULER_H_LSB_ADDR == BNO055_QUATERNION_DATA_W_LSB_ADDR)
  {
    scale = (14);
  }

  bno055_vector_t xyz = {.w = 0, .x = 0, .y = 0, .z = 0};
  if (BNO055_EULER_H_LSB_ADDR == BNO055_QUATERNION_DATA_W_LSB_ADDR)
  {
    xyz.w = (signed short int)((buffer[1] << 8) | buffer[0]) / scale;
    xyz.x = (signed short int)((buffer[3] << 8) | buffer[2]) / scale;
    xyz.y = (signed short int)((buffer[5] << 8) | buffer[4]) / scale;
    xyz.z = (signed short int)((buffer[7] << 8) | buffer[6]) / scale;
  }
  else
  {
    xyz.x = (signed short int)((buffer[1] << 8) | buffer[0]) / scale;
    xyz.y = (signed short int)((buffer[3] << 8) | buffer[2]) / scale;
    xyz.z = (signed short int)((buffer[5] << 8) | buffer[4]) / scale;
  }

  return xyz;
}


int bno055_StepCount(bno055_vector_t myBNO055)
{
  static int stepCount;

  if(abs(myBNO055.z-bno055_InitZ)>=20&&startflag==0)
  {
    timeStart = millis();
    startflag = 1;
  }
  if(abs(myBNO055.z - bno055_InitZ)<=5&&startflag==1)
  {
    timeStop = millis();
    startflag = 0;
    timeinterval = timeStop - timeStart;
  }

  if(timeinterval>=300)
  {
    timeinterval = 0;
    timeStop = 0;
    timeStart = 0;
    ++stepCount;
  }

  return stepCount*2;
}