#ifndef _WIFI_UTILS_HPP_
#define _WIFI_UTILS_HPP_


#include "WiFi.h"
#include "WiFiUdp.h"
#include <Preferences.h>

class WifiUtil
{
    public:
    WifiUtil() {};
    ~WifiUtil() {};

    bool checkServer()
    {
        if(wifiConnected && !foundServer)
            searchServer();
        if(wifiConnected && foundServer)
        {
            if(!client.connected())
            {
                client.stop();
                // delay(500);
                client.setNoDelay(true);
                client.connect(server_ip.c_str(), server_port);
                client.setNoDelay(true);
                PRINTLN("Connecting to the Server: ") 
                PRINTLN(server_ip.c_str() + String(":") + String(server_port));
                delay(500);
                // blink(0, 255, 0, 5, 100);
                if(client.connected())
                {
                    PRINTLN("Connected to the Server");
                    String macAddress = WiFi.macAddress();
                    sendBuffer[0] = 'A';
                    sendBuffer[1] = macAddress.length();
                    //copy the mac address
                    for(int i = 0; i < macAddress.length(); i++)
                        sendBuffer[i + 2] = macAddress[i];
                    client.write(sendBuffer, macAddress.length() + 2);
                    PRINTLN("Sent MAC address: " + macAddress);
                }
                else
                {
                    PRINTLN("Failed to connect to the Server");
                    client.stop();
                    foundServer = false;
                }
                return (client.connected() && (client.available() > 0));
            }
            else
            {
                // PRINT(".");
                // delay(500);
                return client.available() > 0;
            }
        }
        return false;
    }        

    void
    storeCredentials(String ssid, String password)
    {
        //store the credentials in the preferences
        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.end();
    }

    void
    readCredentials(String &ssid, String &password)
    {
        preferences.begin("wifi", true);
        ssid = preferences.getString("ssid", "");
        password = preferences.getString("password", "");
        preferences.end();
    }

    bool
    existsCredentials()
    {
        //check if the credentials are stored
        preferences.begin("wifi", true);
        String ssid = preferences.getString("ssid", "");
        String password = preferences.getString("password", "");
        preferences.end();
        return ssid.length() > 0 && password.length() > 0;
    }

    void
    parseCredentials(String &ssid, String &password)
    {
        //first 2bytes are the size of the ssid and password
        uint8_t size_ssid = Serial.read();
        uint8_t size_password = Serial.read();
        char ssid_buf[size_ssid + 1];
        char password_buf[size_password + 1];
        Serial.readBytes(ssid_buf, size_ssid);
        Serial.readBytes(password_buf, size_password);
        ssid_buf[size_ssid] = '\0';
        password_buf[size_password] = '\0';
        ssid = String(ssid_buf);
        password = String(password_buf);
    }

    void
    connectToWiFi(String ssid, String password, int timeout = 4000)
    {
        WiFi.begin(ssid.c_str(), password.c_str());
        WiFi.setSleep(false);
        unsigned long start = millis();
        while(WiFi.status() != WL_CONNECTED && millis() - start < timeout)
        {
            delay(500);
        }
        if(WiFi.status() == WL_CONNECTED)
        {
            PRINTLN("Connected to the WiFi network");
            wifiConnected = true;
            udp.begin(12345);
        }
        else
        {
            PRINTLN("Failed to connect to the WiFi network");
            PRINTLN("SSID: " + ssid);
            PRINTLN("Password: " + password);
            wifiConnected = false;
        }
    }

    void
    searchServer()
    {
        //clear the UDP buffer
        while(udp.parsePacket() > 0);
        udp.beginPacket("255.255.255.255", 12345); // Broadcast address
        uint8_t msg[] = "MONOMOD";
        udp.write(msg, sizeof(msg));
        udp.endPacket();
        PRINTLN("Searching Server, Broadcasting MONOMOD");
        delay(500);
        int packetSize = udp.parsePacket();
        if(packetSize)
        { 
            server_ip = udp.remoteIP().toString().c_str();
            server_port = udp.remotePort();
            PRINTLN("Received packet of size " + String(packetSize) + " from " +
                    server_ip.c_str() + " on port " + String(server_port));
            char packetBuffer[255];
            int n = udp.read(packetBuffer, 255);
            if(n == 2)
            {
                server_port = (packetBuffer[1] << 8) + packetBuffer[0];
                PRINTLN("Server port: " + String(server_port));
                foundServer = true;
            }
        }
    }

    WiFiClient client;
    bool foundServer = false;
    bool wifiConnected = false;
    private:
    String server_ip;
    int server_port;
    uint32_t dt;
    Preferences preferences;

    WiFiUDP udp;
    uint8_t sendBuffer[1024];
    uint8_t recvBuffer[1024];
    uint8_t spiCmd[255];
};

#endif // _WIFI_UTILS_HPP_