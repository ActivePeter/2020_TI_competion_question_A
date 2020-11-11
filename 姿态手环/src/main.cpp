#include <Arduino.h>
#include "bno080/SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
// #include <Adafruit_ILI9341.h>
#include "Ads_112c04/Ads_112c04.h"
#include "Ads1292/ads1292r.h"
extern "C"
{
#include "bno055/bno055.h"
#include "ssd1306/pa_oled.h"
}
#include <WiFi.h>
//adc通道
#define ANALOG_PIN_0 34

#define TFT_CS 13
#define TFT_DC 12
#define TFT_RST 14
#include "button2/Button2.h"
char buffer[20];

Button2 btn1(35);
Button2 btn2(0);
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Ads_112c04 &ads = Ads_112c04::instance;
ads1292r ads1292;

const char *ssid = "MyESP32AP";
const char *password = "00000000";

bno055_vector_t vec;

WiFiClient client;
const char *host = "192.168.43.1";
const uint16_t port = 1234;
void task(void *pvPar)
{
  uint64_t lastMillis = millis();
  while (1)
  {
    if (millis() - lastMillis > 80)
    {
      lastMillis = millis();
      uint8_t sbuf[9];
      short a = vec.x * 16;
      short b = vec.y * 16;
      short c = vec.z * 16;
      short d = StepCount::stepCount;
      sbuf[0] = 'x';
      sbuf[1] = a >> 8;
      sbuf[2] = a;
      sbuf[3] = b >> 8;
      sbuf[4] = b;
      sbuf[5] = c >> 8;
      sbuf[6] = c;
      sbuf[7] = d >> 8;
      sbuf[8] = d;
      client.write(sbuf, 9);
    }
    // printf("I'm %s\r\n",(char *)pvPar);
    //使用此延时API可以将任务转入阻塞态，期间CPU继续运行其它任务
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
double adcValue = 0;
void adctask(void *pvPar)
{
  while (1)
  {
    adcValue = analogRead(ANALOG_PIN_0);
    adcValue = adcValue * 6.6 / 4096;
    delay(1000);
  }
}

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  // pinMode(ANALOG_PIN_0, INPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("BNO080 Read Example");

  btn1.setPressedHandler([](Button2 &b) {
    Serial.println("btn1");
    StepCount::counting = !StepCount::counting;
  });
  btn2.setPressedHandler([](Button2 &b) {
    Serial.println("btn2");
  });
  // // Serial.begin(115200);
  // // Serial.println();
  // // Serial.println("BNO080 Read Example");
  // // tft.begin();
  // // tft.setRotation(2);
  // // tft.fillScreen(ILI9341_BLUE);

  // ads1292.ads1292_Init();
  Wire.begin();
  Wire.setClock(400000);
  OLED_Init();
  OLED_Clear();
  OLED_ShowString(0, 0, "helloWorld", 12);

  pa_BNO055_init();
  // Wire.setClock(400000); //Increase I2C data rate to 400kHz
  // // pa_BNO055_init();
  // // myIMU.begin();
  StepCount::initFifo();
  // ads.init(Ads_112c04::AxState::DGND, Ads_112c04::AxState::DGND);
  // ads.configRegister0(Ads_112c04::Gain::GAIN_1);
  // delay(100);
  // ads.configRegister1(Ads_112c04::SpeedOfSample::SPS_1000, Ads_112c04::Mode::Mode_Normal, Ads_112c04::ConvMode::Continuous);
  // ads.startConv();

  // myIMU.enableStepCounter(500); //Send data update every 500ms
  // myIMU.enableGyro(50);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  xTaskCreate(task,    //任务函数
              "task1", //这个参数没有什么作用，仅作为任务的描述
              2048,    //任务栈的大小
              NULL,    //传给任务函数的参数
              2,       //优先级，数字越大优先级越高
              NULL);
  // xTaskCreate(adctask, //任务函数
  //             "task2", //这个参数没有什么作用，仅作为任务的描述
  //             2048,    //任务栈的大小
  //             NULL,    //传给任务函数的参数
  //             3,       //优先级，数字越大优先级越高
  //             NULL);
}
char fmqflag=0;
void loop()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    OLED_ShowString(0, 1, "wifi not connected", 12);
  }
  int connect_cnt = 0;
  if (!client.connect(host, port))
  {
    while (!client.connect(host, port))
    {
      delay(500);
      Serial.print("*");
      OLED_ShowString(0, 1, "server not connected", 12);
      connect_cnt++;
      if (connect_cnt > 4)
      {
        return;
      }
    }
    // Serial.println("connection failed");
    return;
  }
  while (1)
  {
    if(fmqflag){
      fmqflag--;
      digitalWrite(13, LOW);
    }else{
      digitalWrite(13, HIGH);
      // delay(50);
      // digitalWrite(13, HIGH);
    }
    if (WiFi.status() != WL_CONNECTED)
    {
      //Serial.println("wifi lost");
      break;
    }

    btn1.loop();
    btn2.loop();
    // Serial.println("BNO080 Read Example");
    vec = pa_BNO055_getVector();

    int stepCount = 0;
    if (pa_BNO055_isLastDataValid())
    {
      stepCount = StepCount::bno055_StepCount(vec);
    }
    {
      char buff[30];
      snprintf(buff, 30, "x %.2f  ", vec.x);
      OLED_ShowString(0, 0, buff, 12);
    }
    {
      char buff[30];
      snprintf(buff, 30, "y %.2f  ", vec.y);
      OLED_ShowString(0, 1, buff, 12);
    }
    {
      char buff[30];
      snprintf(buff, 30, "z %.2f  ", vec.z);
      OLED_ShowString(0, 2, buff, 12);
    }

    {
      // char buff[30];
      // snprintf(buff, 30, caddr_t"", vec.z);
      OLED_ShowString(0, 4, (char *)(StepCount::counting ? "counting" : "stopped  "), 12);
    }
    {
      static int a = 0;
      char buff[30];
      a = stepCount / 10;
      StepCount::distance = (float)(stepCount) * 0.5;
      snprintf(buff, 30, "steps:   %d    ", stepCount);
      OLED_ShowString(0, 5, buff, 16);
      snprintf(buff, 30, "distance:%5.2fm", StepCount::distance);
      OLED_ShowString(0, 7, buff, 12);
    }
    {

      char buff[30];
      snprintf(buff, 30, "ADC:%.2f    ", adcValue);
      OLED_ShowString(0, 3, buff, 12);
    }
  }

  //  Serial.println(line);
  client.stop();
  OLED_Clear();
  // Serial.printf("%f %f %f",vec.x,vec.y,vec.z);
}