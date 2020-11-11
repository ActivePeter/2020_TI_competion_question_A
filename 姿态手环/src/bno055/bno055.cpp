extern "C"
{
#include "bno055.h"
#include "pa_HardwareIIC/pa_HardwareIIC.h"
}
#include "Arduino.h"
#define pa_BNO055_chosenAddress (0x29)

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
//调用此句这前，先启动iic
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

bool pa_BNO055_lastDataValid = true;
bool pa_BNO055_isLastDataValid()
{
  return pa_BNO055_lastDataValid;
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
  // if(xyz.z<0.01&&xyz.z>-0.01){
  //   pa_BNO055_lastDataValid=false;
  //   return xyz;
  // }
  pa_BNO055_lastDataValid = true;
  return xyz;
}
extern char fmqflag;
namespace StepCount
{
  unsigned long timeStart = 0;
  unsigned long timeStop = 0;
  unsigned long timeinterval = 0;
  unsigned char startflag = 0;
  int stepCount;
  float distance;
  bool counting = false;
  const char fifoLen = 5;
  float historyFifo[fifoLen];
  char fifoStep = 0;
  void initFifo()
  {
    memset(historyFifo, 0, fifoLen);
  }
  void pushToFifo(float a)
  {
    historyFifo[fifoStep] = a;
    fifoStep++;
    if (fifoStep == fifoLen)
    {
      fifoStep = 0;
    }
  }
  float getValueFromFifo(int index)
  {
    while (index >= fifoLen)
    {
      index -= fifoLen;
    }
    while (index < 0)
    {
      index += fifoLen;
    }
    return historyFifo[index];
  }

  // int bno055_StepCount(bno055_vector_t myBNO055)
  // {
  //   static char lastDir = 0;
  //   char curDir = 0;
  //   static float lastZ = 0;
  //   if (myBNO055.z > lastZ)
  //   {
  //     curDir = 1;
  //   }

  //   else if (myBNO055.z < lastZ)
  //   {
  //     curDir = -1;
  //   }
  //   if (lastDir == 0)
  //   {
  //     if (curDir != 0)
  //     {
  //       Serial.printf("a\r\n");
  //       fmqflag = 1;
  //       lastDir = curDir;
  //     }
  //   }
  //   else
  //   {
  //     if (curDir == 1 && lastDir == -1)
  //     {
  //       Serial.printf("b\r\n");
  //       lastDir = curDir;
  //     }
  //     else if (curDir == -1 && lastDir == 1)
  //     {
  //       Serial.printf("c\r\n");
  //       lastDir = curDir;
  //     }
  //   }
  //   Serial.printf("%f\r\n",myBNO055.z - lastZ);
  //   lastZ = myBNO055.z;

  //   return stepCount;
  // } // namespace StepCount

  int bno055_StepCount(bno055_vector_t myBNO055)
  {
    pushToFifo(myBNO055.z);

    float valueLast = getValueFromFifo(fifoStep - 1);
    float valueCur = getValueFromFifo(fifoStep - 5);
    static signed char lastDir = 0;
    static uint64_t lastMillis=0;
    if (abs(valueLast - valueCur) > 12)
    {
      signed char curDir = 0;

      if ((valueLast - valueCur) > 0)
      {
        curDir = 1;
      }
      else
      {
        curDir = -1;
      }
      uint64_t curMillis=millis();
      if (lastDir != curDir||abs(curMillis-lastMillis)>2000)
      {

        lastMillis=curMillis;
        Serial.printf("%f %f %f \r\n", valueLast - valueCur, valueLast, valueCur);
        if (counting)
        {

          // Serial.printf("%5.2f %5.2f %5.2f     %5.2f", v1, v2, v3, v2 - v1 + v2 - v3);
          // for (int i = 0; i < fifoLen; i++)
          // {
          //   Serial.printf("%5.2f ", getValueFromFifo(fifoStep + i));
          // }
          ++stepCount;
          fmqflag = 1;
          // digitalWrite(13, LOW);
          // delay(50);
          // digitalWrite(13, HIGH);
        }
      }
      lastDir = curDir;
    }

    // #define StepCountMaxPos 3
    //     // for(int i=0;i<fifoLen;i++){
    //     //   // Serial.printf("%5.2f ",getValueFromFifo(fifoStep + i));
    //     // }
    //     // Serial.println();
    //     char flag = 0;
    //     {
    //       float valueLast = getValueFromFifo(fifoStep);
    //       float valueCur = getValueFromFifo(fifoStep + 1);
    //       flag = valueCur < valueLast;
    //     }
    //     for (int i = 2; i < StepCountMaxPos; i++)
    //     {
    //       float valueLast = getValueFromFifo(fifoStep + i - 1);
    //       float valueCur = getValueFromFifo(fifoStep + i);
    //       // Serial.printf("%d ",valueCur > valueLast);
    //       if (flag)
    //       {
    //         if (valueCur >= valueLast)
    //         {
    //           return stepCount;
    //         }
    //       }
    //       else
    //       {
    //         if (valueCur <= valueLast)
    //         {
    //           return stepCount;
    //         }
    //       }
    //     }
    //     for (int i = StepCountMaxPos; i < fifoLen; i++)
    //     {
    //       float valueLast = getValueFromFifo(fifoStep + i - 1);
    //       float valueCur = getValueFromFifo(fifoStep + i);
    //       // Serial.printf("%d ",valueCur > valueLast);
    //       if (flag)
    //       {
    //         if (valueCur <= valueLast)
    //         {
    //           return stepCount;
    //         }
    //       }
    //       else
    //       {
    //         if (valueCur >= valueLast)
    //         {
    //           return stepCount;
    //         }
    //       }
    //     }
    //     float v1 = getValueFromFifo(fifoStep);
    //     float v2 = getValueFromFifo(fifoStep + StepCountMaxPos);
    //     float v3 = getValueFromFifo(fifoStep + fifoLen - 1);
    //     // float v1 = getValueFromFifo(fifoStep);
    //     // float v2 = getValueFromFifo(fifoStep+StepCountMaxPos);
    //     // float v3 = getValueFromFifo(fifoStep+fifoLen - 1);

    //     if (abs(v2 - v1 + v2 - v3) / 2 < 8)
    //     {
    //       Serial.printf("%5.2f",  v2 - v1 + v2 - v3);
    //       return stepCount;
    //     }

    //     Serial.println();
    //     Serial.println();
    //     if (counting)
    //     {

    //       Serial.printf("%5.2f %5.2f %5.2f     %5.2f", v1, v2, v3, v2 - v1 + v2 - v3);
    //       // for (int i = 0; i < fifoLen; i++)
    //       // {
    //       //   Serial.printf("%5.2f ", getValueFromFifo(fifoStep + i));
    //       // }
    //       ++stepCount;
    //       fmqflag=1;
    //       // digitalWrite(13, LOW);
    //       // delay(50);
    //       // digitalWrite(13, HIGH);
    //     }
    // if (myBNO055.z - bno055_InitZ <= -16 || myBNO055.z - bno055_InitZ >=10 && startflag == 0)
    // {
    //   timeStart = millis();
    //   startflag = 1;
    // }
    // if (abs(myBNO055.z - bno055_InitZ) <= 3 && startflag == 1)
    // {
    //   timeStop = millis();
    //   startflag = 0;
    //   timeinterval = timeStop - timeStart;
    // }

    // if (timeinterval >= 5)
    // {
    //   timeinterval = 0;
    //   timeStop = 0;
    //   timeStart = 0;
    //   if (counting)
    //   {
    //     ++stepCount;
    //   }
    // }

    return stepCount;
  }

  //   int bno055_StepCount(bno055_vector_t myBNO055)
  //   {
  //     pushToFifo(myBNO055.z);
  // #define StepCountMaxPos 3
  //     // for(int i=0;i<fifoLen;i++){
  //     //   // Serial.printf("%5.2f ",getValueFromFifo(fifoStep + i));
  //     // }
  //     // Serial.println();
  //     char flag = 0;
  //     {
  //       float valueLast = getValueFromFifo(fifoStep);
  //       float valueCur = getValueFromFifo(fifoStep + 1);
  //       flag = valueCur < valueLast;
  //     }
  //     for (int i = 2; i < StepCountMaxPos; i++)
  //     {
  //       float valueLast = getValueFromFifo(fifoStep + i - 1);
  //       float valueCur = getValueFromFifo(fifoStep + i);
  //       // Serial.printf("%d ",valueCur > valueLast);
  //       if (flag)
  //       {
  //         if (valueCur >= valueLast)
  //         {
  //           return stepCount;
  //         }
  //       }
  //       else
  //       {
  //         if (valueCur <= valueLast)
  //         {
  //           return stepCount;
  //         }
  //       }
  //     }
  //     for (int i = StepCountMaxPos; i < fifoLen; i++)
  //     {
  //       float valueLast = getValueFromFifo(fifoStep + i - 1);
  //       float valueCur = getValueFromFifo(fifoStep + i);
  //       // Serial.printf("%d ",valueCur > valueLast);
  //       if (flag)
  //       {
  //         if (valueCur <= valueLast)
  //         {
  //           return stepCount;
  //         }
  //       }
  //       else
  //       {
  //         if (valueCur >= valueLast)
  //         {
  //           return stepCount;
  //         }
  //       }
  //     }
  //     float v1 = getValueFromFifo(fifoStep);
  //     float v2 = getValueFromFifo(fifoStep + StepCountMaxPos);
  //     float v3 = getValueFromFifo(fifoStep + fifoLen - 1);
  //     // float v1 = getValueFromFifo(fifoStep);
  //     // float v2 = getValueFromFifo(fifoStep+StepCountMaxPos);
  //     // float v3 = getValueFromFifo(fifoStep+fifoLen - 1);

  //     if (abs(v2 - v1 + v2 - v3) / 2 < 8)
  //     {
  //       Serial.printf("%5.2f",  v2 - v1 + v2 - v3);
  //       return stepCount;
  //     }

  //     Serial.println();
  //     Serial.println();
  //     if (counting)
  //     {

  //       Serial.printf("%5.2f %5.2f %5.2f     %5.2f", v1, v2, v3, v2 - v1 + v2 - v3);
  //       // for (int i = 0; i < fifoLen; i++)
  //       // {
  //       //   Serial.printf("%5.2f ", getValueFromFifo(fifoStep + i));
  //       // }
  //       ++stepCount;
  //       fmqflag=1;
  //       // digitalWrite(13, LOW);
  //       // delay(50);
  //       // digitalWrite(13, HIGH);
  //     }
  //     // if (myBNO055.z - bno055_InitZ <= -16 || myBNO055.z - bno055_InitZ >=10 && startflag == 0)
  //     // {
  //     //   timeStart = millis();
  //     //   startflag = 1;
  //     // }
  //     // if (abs(myBNO055.z - bno055_InitZ) <= 3 && startflag == 1)
  //     // {
  //     //   timeStop = millis();
  //     //   startflag = 0;
  //     //   timeinterval = timeStop - timeStart;
  //     // }

  //     // if (timeinterval >= 5)
  //     // {
  //     //   timeinterval = 0;
  //     //   timeStop = 0;
  //     //   timeStart = 0;
  //     //   if (counting)
  //     //   {
  //     //     ++stepCount;
  //     //   }
  //     // }

  //     return stepCount;
  //   }

} // namespace StepCount
