#ifndef __NETWORK_H__
#define __NETWORK_H__
#include "Arduino.h"
#include "WiFi.h"
namespace Network
{
    void sendTask(void *pvPar);
    void beginTask();
    extern WiFiClient client;
    extern const char *host;
    extern const uint16_t port;
    extern bool heartCollected;
    extern bool tempCollected;
} // namespace Network

#endif // __NETWORK_H__