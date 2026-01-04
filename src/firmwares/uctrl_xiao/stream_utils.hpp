#ifndef _STREAM_UTILS_HPP_
#define _STREAM_UTILS_HPP_

#include <Arduino.h>
#include "wifi_utilis.hpp"

class MyStream
{
    enum
    {
        isNONE = 0,
        isSERIAL = 1,
        isCLIENT = 2
    };

    public:
    MyStream() {}
    int
    available()
    {
        if(Serial.available() > 0)
        {
            m_active = isSERIAL;
            return Serial.available();
        }
        else if(m_wifiUtil->checkServer())
        {
            m_active = isCLIENT;
            return m_client->available();
        }
        else
        {
            if(!m_wifiUtil->foundServer)
                stream_active = false;
            
            m_active = isNONE;
            return 0;
        }
    }

    int
    readBytes(char *buffer, int size)
    {
        if(m_active == isSERIAL)
        {
            return Serial.readBytes(buffer, size);
        }
        else if(m_active == isCLIENT)
        {
            return m_client->readBytes(buffer, size);
        }
        else
        {
            return 0;
        }
    }
    int
    write(uint8_t *buffer, int size)
    {
        if(m_active == isSERIAL)
        {
            return Serial.write(buffer, size);
        }
        else if(m_active == isCLIENT)
        {
            return m_client->write(buffer, size);
        }
        else
        {
            return 0;
        }
    }

    void
    setWifiUtil(WifiUtil *wifiUtil)
    {
        m_wifiUtil = wifiUtil;
        m_client = &m_wifiUtil->client;
        m_client->setTimeout(1000);
    }

    void setWifi()
    {
        m_active = isCLIENT;
    }
    void setSerial()
    {
        m_active = isSERIAL;
    }
    void setNone()
    {
        m_active = isNONE;
    }
    void toggle()
    {
        if(m_active == isSERIAL)
        {
            m_wifiUtil->searchServer();
            m_active = isCLIENT;
        }
        else if(m_active == isCLIENT)
        {
            m_active = isSERIAL;
        }
    }


    bool stream_active = false;
    private:
    WifiUtil *m_wifiUtil;
    WiFiClient *m_client;
    int m_active = isNONE;

};

#endif // _STREAM_UTILS_HPP_