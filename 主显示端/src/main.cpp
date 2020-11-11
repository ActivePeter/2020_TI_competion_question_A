#include <Arduino.h>
#include "bno080/SparkFun_BNO080_Arduino_Library.h"
#include <Adafruit_ILI9341.h>
#include "Ads_112c04/Ads_112c04.h"
#include "touchScreen/pa_touchScreen.h"
extern "C"
{
#include "bno055/bno055.h"
}
#include "pa_miniGUI/pa_button/pa_Button.h"

#include "network/network.h"

#define TFT_CS 13
#define TFT_DC 12
#define TFT_RST 14

#define Attitude 1
#define Ads1292 2
#define Lm70 3
#define TempOffset 204.393 - 5.1
#define Normal 0
#define Fever 1
#define HighFever 2

#define HEAD_START_1 0x0A
#define HEAD_START_2 0xFA

namespace Btn
{
  extern pa_Button Btn_switch;
  extern pa_Button Btn_ads;
  extern pa_Button Btn_lm70;
  extern pa_Button Btn_attitude;
  extern pa_Button Btn_tempPlus;
  extern pa_Button Btn_tempMinus;

  extern pa_Button Btn_tempConfirm;

  void adsCallback();
  void lm70Callback();
  void attitudeCallback();
  void switchCallback();
}; // namespace Btn

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

char buffer[4];
uint32_t heartRateSend = 0;
char frameHeader[2] = {0x0a, 0xfa};
BNO080 myIMU;
bno055_vector_t myBNO055;
float bno055_InitZ = 0;
unsigned char a = 50;
unsigned char humanBodyState = 0;
int stepCount = 0;
double temperature = 0;
double temperatureGet = 0;
double temperatureOffset = 0;
bool startTemp = false;

bool startHeart = false;
uint64_t startHeartTime = 0;

double initialTemperature = 0;
double handleTemperature[20];
double temperatureSum = 0;
int cnt_ofMinus = 0;
int heartRateValue = 0;
int numdata = 0;
char buf = 0;
const char *ssid = "MyESP32AP";
const char *password = "00000000";
unsigned char tempMeasureStop = 0;
Ads_112c04 &ads = Ads_112c04::instance;

double Attitude_cache[180] = {0};
double Ads1292_cache[360] = {0};
double Lm70_cache[180] = {0};

void waveformDisplay(int16_t x, int16_t y, double data, double dataMax, double dataMin, unsigned char subsection, unsigned device, unsigned steps); //画坐标位置 27<=x<=62

bool pa_Button::isPressed()
{
  return pa_touchScreen::instance.isPressed();
}
namespace Heart_Namespace
{
  int collectCnt = 0;
  float collectSum = 0;

  float Momentfrequence = 0;
  float avrFreq = 0;
#define Heart_cache_len 720
  float cache[Heart_cache_len] = {0};
  int cacheStep = 0;
  short maxValueIndex = 0;
  short minValueIndex = 0;
  uint16_t deltaMillis = 0;
  float getCacheDataByIndex(int index)
  {
    while (index < 0)
    {
      index += Heart_cache_len;
    }
    index %= Heart_cache_len;

    return cache[index];
  }
  int getMinValue()
  {
    return cache[minValueIndex];
  }
  int getMaxValue()
  {
    return cache[maxValueIndex];
  }
  int findMaxIndex()
  {
    int maxindex = 0;
    for (int i = 1; i < Heart_cache_len; i++)
    {
      if (cache[i] > cache[maxindex])
      {
        maxindex = i;
      }
    }
    return maxindex;
  }
  int findMinIndex()
  {
    int minindex = 0;
    for (int i = 1; i < Heart_cache_len; i++)
    {
      if (cache[i] < cache[minindex])
      {
        minindex = i;
      }
    }
    return minindex;
  }
  void pushDataToCache(double data)
  {
    if (data > cache[maxValueIndex])
    {
      maxValueIndex = cacheStep;
    }
    else
    {
      if (cacheStep == maxValueIndex)
      {
        cache[cacheStep] = data;
        maxValueIndex = findMaxIndex();
      }
    }
    if (data < cache[minValueIndex])
    {
      minValueIndex = cacheStep;
    }
    else
    {
      if (cacheStep == minValueIndex)
      {
        cache[cacheStep] = data;
        minValueIndex = findMinIndex();
      }
    }
    cache[cacheStep] = data;
    // if()
    cacheStep++;
    if (cacheStep == Heart_cache_len)
    {
      cacheStep = 0;
    }
  }
  const char heartBeatMillisBuffLen = 80;
  uint16_t heartBeatMillisBuff[heartBeatMillisBuffLen];
  char heartBeatMillisStep = 0;
  int getHeartBeatMillisByIndex(int index)
  {
    while (index < 0)
    {
      index += heartBeatMillisBuffLen;
    }
    index %= heartBeatMillisBuffLen;
    return heartBeatMillisBuff[index];
  }
  void pushHeartBeatMillisToBuff(uint16_t millis)
  {
    heartBeatMillisBuff[heartBeatMillisStep] = millis;
    heartBeatMillisStep++;
    if (heartBeatMillisStep == heartBeatMillisBuffLen)
    {
      heartBeatMillisStep = 0;
    }
  }
  void updateMomentFrequence()
  {
    float momentf = 0;
    for (int i = 0; i < 3; i++)
    {
      momentf += getHeartBeatMillisByIndex(heartBeatMillisStep - i - 1);
    }
    Momentfrequence = 3000 * 60 / momentf;
    if (startHeart)
    {
      collectCnt++;
      collectSum += Momentfrequence;
      if (millis() - startHeartTime > 10000)
      {
        startHeart = false;
        avrFreq = collectSum / collectCnt;
        Network::heartCollected = true;
      }
    }
    else
    {
      collectCnt = 0;
      collectSum = 0;
    }
  }
  void readFromSerial()
  {
    while (Serial.available())
    {
      static unsigned char numcount = 0;
      if (numcount < 2)
      {
        buf = Serial.read();
        // Serial.printf("%c\r\n",buf);
        if (numcount == 0)
        {
          if (HEAD_START_1 == buf)
          {
            ++numcount;
          }
        }
        if (numcount == 1)
        {
          if (HEAD_START_2 == buf)
          {
            ++numcount;
          }
        }
      }
      if (numcount == 2)
      {
        numdata = Serial.readBytes(buffer, 4);
        numcount = 0;
      }
      if (numdata == 4)
      {
        short value = buffer[0] | (uint16_t)buffer[1] << 8;
        deltaMillis = buffer[2] | (uint16_t)buffer[3] << 8;
        pushHeartBeatMillisToBuff(deltaMillis);
        updateMomentFrequence();
        heartRateValue = -value; //| (uint32_t)buffer[2] << 16 | (uint32_t)buffer[3] << 24;
        heartRateSend = heartRateValue;
        static int lastHeartRateValue = 0;

        // Serial.printf("heartRate %d\r\n", heartRateValue-lastHeartRateValue);

        lastHeartRateValue = heartRateValue;
        pushDataToCache(heartRateValue);
      }
    }

    // int sum = 0;
    // static int lastSum = 0;
    // signed char dir = 0;
    // static signed char lastDir = 0;
    // for (int i = 0; i < 5; i++)
    // {
    //   sum += Heart_Namespace::getCacheDataByIndex(Heart_Namespace::cacheStep - i);
    // }

    // if (sum > 20)
    // {
    //   if (sum > lastSum)
    //   {
    //     dir = 1;
    //   }
    //   if (sum < lastSum)
    //   {
    //     dir = -1;
    //   }
    //   if (lastDir == 1 && dir == -1)
    //   {
    //     static uint64_t lastHeartBeatMillis = 0;
    //     short deltaBeatTime = millis() - lastHeartBeatMillis;
    //     pushHeartBeatMillisToBuff(deltaBeatTime);
    //     updateMomentFrequence();
    //     Serial.printf("heartRate %d %d %d\r\n", deltaBeatTime, lastDir, dir);
    //     lastHeartBeatMillis = millis();
    //   }
    // }

    // lastSum = sum;
    // if (dir != 0)
    // {
    //   lastDir = dir;
    // }
  }
} // namespace Heart_Namespace

namespace GUI
{
  const char BuffWidth = 180;
  const char BuffHeight = 90;
  uint16_t drawBuff[BuffWidth * BuffHeight];

  void clearDrawBuff()
  {
    for (int i = 0; i < 180 * 90; i++)
    {
      drawBuff[i] = ILI9341_BLACK;
    }
    // memset(drawBuff,0,sizeof(drawBuff));
  }
  void drawPointInBuff(int x, int y, uint16_t color)
  {
    if (x < 0)
      x = 0;
    if (x > BuffWidth - 1)
      x = BuffWidth - 1;
    if (y < 0)
      y = 0;
    if (y > BuffHeight - 1)
      y = BuffHeight - 1;

    drawBuff[y * BuffWidth + x] = color;
  }
  void drawLineInBuff(int x, int y, int x1, int y1, uint16_t color)
  {
    if (x < 0)
      x = 0;
    if (x > BuffWidth - 1)
      x = BuffWidth - 1;
    if (y < 0)
      y = 0;
    if (y > BuffHeight - 1)
      y = BuffHeight - 1;
    if (x1 < 0)
      x1 = 0;
    if (x1 > BuffWidth - 1)
      x1 = BuffWidth - 1;
    if (y1 < 0)
      y1 = 0;
    if (y1 > BuffHeight - 1)
      y1 = BuffHeight - 1;
    int dx = x - x1;
    int dy = y - y1;

    int distance = sqrt(dx * dx + dy * dy);
    for (int i = 0; i < distance + 1; i++)
    {
      drawPointInBuff(x1 + i * dx / distance, y1 + i * dy / distance, color);
    }
  }
  void writeBuffToScreen(int x, int y)
  {
    tft.startWrite();
    tft.setAddrWindow(x, y, BuffWidth, BuffHeight);    // Clipped area
    tft.writePixels(drawBuff, BuffWidth * BuffHeight); // Push one (clipped) row
    tft.endWrite();
    // int i=0;
    // tft.setCursor();
    // tft.writePixels(drawBuff,180*90);
  }
  void drawButton(int x, int y, int w, int h, const char *str)
  {
    tft.drawRoundRect(x, y, w, h, 4, ILI9341_WHITE);
    tft.setCursor(x + 10, y + 5);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.printf(str, stepCount);
  }
  void drawButton2(int x, int y, int w, int h, int value)
  {
    tft.drawRoundRect(x, y, w, h, 4, ILI9341_GREEN);
    tft.setCursor(x + 10, y + 5);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.printf("%d", value);
  }
  void gui_Pos()
  {
    tft.setCursor(50, 17);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.printf("Attitude\n");
    tft.println();

    // tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    // tft.printf("X:%.2lf\n", myBNO055.x);
    // tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    // tft.printf("Y:%.2lf\n", myBNO055.y);
    // tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    // tft.printf("Z:%.2lf\n", myBNO055.z);
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    tft.printf("stepCount:%d\n", stepCount);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.printf("distance:%.1f m\n", stepCount * 0.5);

    tft.drawRoundRect(160, 10, 50, 30, 4, ILI9341_WHITE);
    tft.setCursor(174, 17);
    tft.setTextColor(ILI9341_WHITE);
    tft.printf(">>", stepCount);
    // waveformDisplay(36, 151, myBNO055.z, 180, 0, 5, Attitude, 1);
  }
  void gui_ads1292()
  {
    tft.setCursor(50, 17);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.printf("ADS1292\n");
    tft.println();
    tft.printf("momentF %.2f bpm \n", Heart_Namespace::Momentfrequence);
    tft.println();
    tft.printf("avrFreq %.2f bpm \n", Heart_Namespace::avrFreq);
    tft.println();

    if (startHeart)
    {
      drawButton2(160, 260, 40, 40, (millis() - startHeartTime) / 1000);
    }
    else
    {
      drawButton(160, 260, 40, 40, "on");
    }

    tft.drawRoundRect(160, 10, 50, 30, 4, ILI9341_WHITE);
    tft.setCursor(174, 17);
    tft.setTextColor(ILI9341_WHITE);
    tft.printf(">>", stepCount);
    waveformDisplay(36, 151, heartRateValue, Heart_Namespace::getMaxValue(), Heart_Namespace::getMinValue(), 5, Ads1292, 2);
  }
  void gui_lm70()
  {
    tft.setCursor(70, 17);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.printf("LM70\n");
    tft.println();
    tft.printf("Temp:\n");
    tft.println();
    tft.printf("State:\n");
    tft.println();
    tft.printf("Offset:%.1f", temperatureOffset);
    drawButton(20, 260, 40, 40, "+");
    drawButton(80, 260, 40, 40, "-");
    if (startTemp)
    {
      drawButton2(160, 260, 40, 40, cnt_ofMinus);
    }
    else
    {
      drawButton(160, 260, 40, 40, "on");
    }

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(160, 52);
    tft.printf("%.2f C\n", temperatureGet);

    switch (humanBodyState)
    {
    case Normal:
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
      tft.setCursor(70, 52);
      tft.printf("%.2f C\n", temperature);
      tft.setCursor(78, 82);
      tft.printf("Health    ");
      break;
    case Fever:
      tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
      tft.setCursor(70, 52);
      tft.printf("%.2f C\n", temperature);
      tft.setCursor(78, 82);
      tft.printf("Fever    ");
      break;
    case HighFever:
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      tft.setCursor(70, 52);
      tft.printf("%.2f C\n", temperature);
      tft.setCursor(78, 82);
      tft.printf("HighFever");
      break;
    }

    tft.drawRoundRect(160, 10, 50, 30, 4, ILI9341_WHITE);
    tft.setCursor(174, 17);
    tft.setTextColor(ILI9341_WHITE);
    tft.printf(">>", stepCount);
    waveformDisplay(36, 151, temperature, 45, 20, 5, Lm70, 1);
  }

  void gui_Menu()
  {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(90, 30);
    tft.printf("Menu");
    drawButton(60, 60, 120, 30, "Ads1292");
    drawButton(60, 120, 120, 30, "Lm70");
    drawButton(60, 180, 120, 30, "Attitude");
  }
#define GUI_chosen_menu 0
#define GUI_chosen_pos 1
#define GUI_chosen_ads1292 2
#define GUI_chosen_lm70 3
  int GUI_Chosen = 0;

  void loop()
  {
    if (GUI_Chosen == GUI_chosen_menu)
    {
      Btn::Btn_ads.loop();
      Btn::Btn_attitude.loop();
      Btn::Btn_lm70.loop();
    }
    else
    {
      if (GUI_Chosen == GUI_chosen_lm70)
      {
        Btn::Btn_tempPlus.loop();
        Btn::Btn_tempMinus.loop();
        Btn::Btn_tempConfirm.loop();
      }
      if (GUI_Chosen == GUI_chosen_ads1292)
      {
        Btn::Btn_tempConfirm.loop();
      }
      Btn::Btn_switch.loop();
    }
    switch (GUI_Chosen)
    {
    case GUI_chosen_pos:
      gui_Pos();
      break;
    case GUI_chosen_menu:
      gui_Menu();
      break;
    case GUI_chosen_lm70:
      gui_lm70();
      break;
    case GUI_chosen_ads1292:
      gui_ads1292();
      break;
    }
  }
} // namespace GUI

namespace Btn
{
  pa_Button Btn_tempPlus;
  pa_Button Btn_tempMinus;

  pa_Button Btn_tempConfirm;

  pa_Button Btn_switch;
  pa_Button Btn_ads;
  pa_Button Btn_lm70;
  pa_Button Btn_attitude;

  void Btn_tempConfirmCallBack()
  {
    if (GUI::GUI_Chosen == GUI_chosen_lm70)
    {
      startTemp = true;
    }
    else if (GUI::GUI_Chosen == GUI_chosen_ads1292)
    {
      startHeart = true;
      startHeartTime = millis();
    }
  }

  void Btn_tempPlusCallBack()
  {
    temperatureOffset += 0.1;
  }
  void Btn_tempMinusCallBack()
  {
    temperatureOffset -= 0.1;
  }

  void adsCallback()
  {
    Serial.println("ads");
    GUI::GUI_Chosen = GUI_chosen_ads1292;
    tft.fillScreen(ILI9341_BLACK);
  }
  void lm70Callback()
  {
    Serial.println("lm70");
    GUI::GUI_Chosen = GUI_chosen_lm70;
    tft.fillScreen(ILI9341_BLACK);
  }
  void attitudeCallback()
  {
    Serial.println("attitude");
    GUI::GUI_Chosen = GUI_chosen_pos;
    tft.fillScreen(ILI9341_BLACK);
  }
  void switchCallback()
  {
    Serial.println("switch");
    GUI::GUI_Chosen = GUI_chosen_menu;
    tft.fillScreen(ILI9341_BLACK);
  }
} // namespace Btn

void setup()
{
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  Serial.begin(115200);
  Serial.println();
  Serial.println("HELLO");
  pa_touchScreen::instance.init(240, 320, 52748, 56370, 425, 3780, 100);

  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);

  Wire.begin();

  // pa_BNO055_init();

  ads.init(Ads_112c04::AxState::DGND, Ads_112c04::AxState::DGND);
  ads.configRegister0(Ads_112c04::Gain::GAIN_1);
  delay(100);
  ads.configRegister1(Ads_112c04::SpeedOfSample::SPS_1000, Ads_112c04::Mode::Mode_Normal, Ads_112c04::ConvMode::Continuous);
  ads.startConv();

  Wire.setClock(400000); //Increase I2C data rate to 400kHz

  Btn::Btn_switch.init(160, 10, 50, 30);
  Btn::Btn_ads.init(60, 60, 120, 30);
  Btn::Btn_lm70.init(60, 120, 120, 30);
  Btn::Btn_attitude.init(60, 180, 120, 30);
  Btn::Btn_tempPlus.init(20, 260, 40, 40);
  Btn::Btn_tempMinus.init(80, 260, 40, 40);
  Btn::Btn_tempConfirm.init(160, 260, 40, 40);

  Btn::Btn_ads.buttonCallback = Btn::adsCallback;
  Btn::Btn_attitude.buttonCallback = Btn::attitudeCallback;
  Btn::Btn_lm70.buttonCallback = Btn::lm70Callback;
  Btn::Btn_switch.buttonCallback = Btn::switchCallback;
  Btn::Btn_tempMinus.buttonCallback = Btn::Btn_tempMinusCallBack;
  Btn::Btn_tempPlus.buttonCallback = Btn::Btn_tempPlusCallBack;
  Btn::Btn_tempConfirm.buttonCallback = Btn::Btn_tempConfirmCallBack;

  //wifi setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(1);
    tft.fillScreen(ILI9341_BLACK);
    tft.print("wifi not connected");
    Serial.print(".");
    WiFi.begin(ssid, password);
    delay(500);
  }
  Network::beginTask();
}

void loop()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(1);
    tft.fillScreen(ILI9341_BLACK);
    tft.print("wifi not connected");
  }
  int connect_cnt = 0;
  Network::client.connect(Network::host, Network::port);
  // if (!Network::client.connect(Network::host, Network::port))
  // {
  //   while (!Network::client.connect(Network::host, Network::port))
  //   {
  //     delay(500);
  //     tft.setCursor(0, 0);
  //     tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  //     tft.setTextSize(1);
  //     tft.fillScreen(ILI9341_BLACK);
  //     tft.print("server not connected");
  //     connect_cnt++;
  //     if (connect_cnt > 4)
  //     {
  //       return;
  //     }
  //   }
  //   // Serial.println("connection failed");
  //   return;
  // }
  tft.fillScreen(ILI9341_BLACK);
  while (1)
  {
    uint16_t coord[2];
    pa_touchScreen::instance.readRaw(coord);
    // Serial.printf("%d %d\r\n",coord[0],coord[1]);
    pa_touchScreen::instance.turnRawToScreen(coord);
    pa_Button::setPos(coord[0], coord[1]);

    double adc = ads.readADC(); //获取112c04的值
    adc = (float)adc * (2.048 / 32768) * 1000;
    // Serial.printf("%F \r\n",adc);
    initialTemperature = (-0.0000084515) * adc * adc + (-0.176928) * adc + TempOffset + temperatureOffset; //数据拟合
    //均值滤波
    static unsigned char j = 0;
    handleTemperature[j] = initialTemperature;
    j++;
    j %= 20;
    temperatureSum = 0;
    for (unsigned char i = 0; i < 20; i++)
    {
      temperatureSum += handleTemperature[i];
    }
    temperature = temperatureSum / 20;
#define tempArrayLen 30
    static double temperatureArry[tempArrayLen];
    static unsigned char temperature_index = 0;
    temperatureArry[temperature_index] = temperature;
    temperature_index++;
    temperature_index %= tempArrayLen;

    if (temperature_index == tempArrayLen - 1)
    {
      // Serial.printf("t %.2f\r\n", temperatureArry[tempArrayLen - 2] - temperatureArry[tempArrayLen - 1]);

      if (startTemp == true)
      {
        if (temperatureArry[tempArrayLen - 2] - temperatureArry[tempArrayLen - 1] < 0)
        {
          cnt_ofMinus++;
        }
        if (cnt_ofMinus > 3)
        {
          temperatureGet = temperature;
          Network::tempCollected = true;
          startTemp = false;
        }
      }
      else
      {
        cnt_ofMinus = 0;
      }
      // if ((temperatureArry[tempArrayLen-2] - temperatureArry[0]) <= 0.5)
      // {
      //   tempMeasureStop = 1;
      // }
    }

    //温度判断人状态
    if (temperature >= 34.5 && temperature < 37)
    {
      humanBodyState = Fever;
    }
    else if (temperature >= 37)
    {
      humanBodyState = HighFever;
    }
    else
    {
      humanBodyState = Normal;
    }
    //获取055角度值
    myBNO055 = pa_BNO055_getVector();
    if (a)
    {
      bno055_InitZ = myBNO055.z;
      --a;
    }
    //计算步数
    // stepCount = bno055_StepCount(myBNO055);
    //获取心率值
    // Heart_Namespace::readFromSerial();
    // heartRateValue
    //gui
    GUI::loop();
  }
  Network::client.stop();
  // OLED_Clear();
}

//36 151 180 90
void drawLineInRange(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  if (y0 < 151)
  {
    y0 = 151;
  }
  if (y0 > 151 + 90)
  {
    y0 = 151 + 90;
  }
  if (y1 < 151)
  {
    y1 = 151;
  }
  if (y1 > 151 + 90)
  {
    y1 = 151 + 90;
  }
  tft.drawLine(x0, y0, x1, y1, color);
}
//波形显示
void waveformDisplay(int16_t x, int16_t y, double data, double dataMax, double dataMin, unsigned char subsection, unsigned device, unsigned steps)
{
  Serial.println();
  static unsigned char Attitude_count = 0;
  // static unsigned char Ads1292_count = 0;
  // static unsigned char Ads1292_index = 0;
  static unsigned char Lm70_count = 0;

  unsigned char i;

  Serial.println();
  //更新数据线
  switch (device)
  {
  case Attitude:
    Attitude_cache[Attitude_count] = abs((data - dataMin) * 90 / (dataMax - dataMin));
    // int x1=x + Attitude_count*steps;
    // int x2=
    drawLineInRange(x + Attitude_count + steps, y, x + Attitude_count + steps, y + 90, ILI9341_BLUE); //蓝色前刷新线
    drawLineInRange(x + Attitude_count, y, x + Attitude_count, y + 90, ILI9341_BLACK);                //黑色后刷新线
    if (Attitude_cache[Attitude_count] < 90)
    {
      drawLineInRange(x + Attitude_count, y + 90 - Attitude_cache[Attitude_count - 1], x + Attitude_count + 1, y + 90 - Attitude_cache[Attitude_count], ILI9341_RED); //数据线  241~151
    }
    drawLineInRange(x + 180, y, x + 180, y + 90, ILI9341_BLACK); //数据线  241~151
    Attitude_count += steps;
    Attitude_count %= 180;
    break;
  case Ads1292:
  {

    GUI::clearDrawBuff();
    Serial.println();
    for (int i = 0; i < 179; i++)
    {

      float y1 = Heart_Namespace::getCacheDataByIndex(i * 4);
      float y2 = Heart_Namespace::getCacheDataByIndex((i + 1) * 4);
      y1 = 90 - ((y1 - dataMin) * 90 / (dataMax - dataMin));
      y2 = 90 - ((y2 - dataMin) * 90 / (dataMax - dataMin));
      GUI::drawLineInBuff(i, y1, i + 1, y2, ILI9341_RED);
    }
    GUI::drawLineInBuff(Heart_Namespace::cacheStep / 4, 0, Heart_Namespace::cacheStep / 4, 90, ILI9341_GREEN);
    Serial.println();
    GUI::writeBuffToScreen(x, y);
    Serial.println();
    // tft.write
    // tft.drawBitmap(x,y,GUI::drawBuff,180,90,ILI9341_BLACK);
    // tft.drawBitmap
  }
  break;
  case Lm70:
    Lm70_cache[Lm70_count] = abs((data - dataMin) * 90 / (dataMax - dataMin));
    drawLineInRange(x + Lm70_count + steps, y, x + Lm70_count + steps, y + 90, ILI9341_BLUE); //蓝色前刷新线
    drawLineInRange(x + Lm70_count, y, x + Lm70_count, y + 90, ILI9341_BLACK);
    switch (humanBodyState)
    {
    case Normal:
      if (Lm70_cache[Lm70_count] < 90)
      {
        drawLineInRange(x + Lm70_count, y + 90 - Lm70_cache[Lm70_count - 1], x + Lm70_count + 1, y + 90 - Lm70_cache[Lm70_count], ILI9341_GREEN); //数据线  241~151
      }

      break;
    case Fever:
      if (Lm70_cache[Lm70_count] < 90)
      {
        drawLineInRange(x + Lm70_count, y + 90 - Lm70_cache[Lm70_count - 1], x + Lm70_count + 1, y + 90 - Lm70_cache[Lm70_count], ILI9341_YELLOW); //数据线  241~151
      }
      break;
    case HighFever:
      if (Lm70_cache[Lm70_count] < 90)
      {
        drawLineInRange(x + Lm70_count, y + 90 - Lm70_cache[Lm70_count - 1], x + Lm70_count + 1, y + 90 - Lm70_cache[Lm70_count], ILI9341_RED); //数据线  241~151
      }
      break;
    }
    drawLineInRange(x + 180, y, x + 180, y + 90, ILI9341_BLACK);
    Lm70_count += steps;
    Lm70_count %= 180;
    break;
  }
  //画坐标系、标刻度
  tft.drawLine(x, y - 8, x, y + 2 + 90, ILI9341_BLUE);            //竖轴
  tft.drawLine(x, y + 2 + 90, x + 180, y + 2 + 90, ILI9341_BLUE); //横轴
  for (i = 0; i < subsection + 1; i++)
  {
    tft.setCursor(x - 36, y - 2 + i * (90 / subsection));
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(1);
    tft.printf("%.0f", (dataMax - dataMin) / subsection * (subsection - i) + dataMin);
    tft.drawLine(x - 2, y + i * (90 / subsection), x, y + i * (90 / subsection), ILI9341_WHITE); //标刻度
  }
}