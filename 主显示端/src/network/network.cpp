#include "network.h"

#include <WiFi.h>
extern double temperature;
extern double temperatureGet;
extern uint32_t heartRateSend;
namespace Heart_Namespace
{
    extern float Momentfrequence;
    extern uint16_t deltaMillis;
    extern float avrFreq;
    void readFromSerial();
} // namespace Heart_Namespace

extern int stepCount;
namespace Network
{

    bool heartCollected = false;
    bool tempCollected = false;

    WiFiClient client;
    const char *host = "192.168.43.1";
    const uint16_t port = 1234;

    void beginTask()
    {
        xTaskCreate(sendTask, //任务函数
                    "task1",  //这个参数没有什么作用，仅作为任务的描述
                    2048,     //任务栈的大小
                    NULL,     //传给任务函数的参数
                    2,        //优先级，数字越大优先级越高
                    NULL);
    }

    void sendTask(void *pvPar)
    {
        uint64_t lastMillis = millis();
        while (1)
        {

            if (millis() - lastMillis > 20)
            {
                Heart_Namespace::readFromSerial();
                if (client.available() >= 3)
                {
                    Serial.printf("ggg\r\n");
                    uint8_t rbuf[3];
                    if (client.readBytes(rbuf, 3) >= 3)
                    {
                        Serial.write(rbuf, 3);
                        Serial.println();
                        Serial.printf("ggg\r\n");
                        if (rbuf[0] == 't')
                        {
                            stepCount = (uint16_t)rbuf[1] << 8 | rbuf[2];
                        }
                    }
                }
                lastMillis = millis();
                uint8_t sbuf[11];
                short a = temperature * 100;
                uint16_t b = Heart_Namespace::Momentfrequence * 100;
                uint16_t c = Heart_Namespace::deltaMillis;
                sbuf[0] = 't';
                sbuf[1] = a >> 8;
                sbuf[2] = a;
                sbuf[3] = heartRateSend >> 24;
                sbuf[4] = heartRateSend >> 16;
                sbuf[5] = heartRateSend >> 8;
                sbuf[6] = heartRateSend;
                sbuf[7] = b >> 8;
                sbuf[8] = b;
                sbuf[9] = c >> 8;
                sbuf[10] = c;
                // sbuf[7] = d>>8;
                // sbuf[8] = d;
                client.write(sbuf, sizeof(sbuf));

                if (heartCollected)
                {
                    heartCollected = false;
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    b = Heart_Namespace::avrFreq * 100;
                    sbuf[0] = 'z';

                    sbuf[1] = b >> 8;
                    sbuf[2] = b;
                    client.write(sbuf, sizeof(sbuf));
                }
                if (tempCollected)
                {
                    tempCollected = false;
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    b = temperatureGet * 100;
                    sbuf[0] = 'y';

                    sbuf[1] = b >> 8;
                    sbuf[2] = b;
                    client.write(sbuf, sizeof(sbuf));
                }
            }
            // printf("I'm %s\r\n",(char *)pvPar);
            //使用此延时API可以将任务转入阻塞态，期间CPU继续运行其它任务
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
} // namespace Network