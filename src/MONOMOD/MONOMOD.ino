#include "WiFi.h"
#include "WiFiUdp.h"
#include <Adafruit_NeoPixel.h>
#define DEBUG 0
#define PRINT(s)         \
    if(DEBUG)            \
    {                    \
        Serial.print(s); \
    }
#define PRINTLN(s)         \
    if(DEBUG)              \
    {                      \
        Serial.println(s); \
    }

#define PIN_LED 6
#define NB_LED 2
#include <Preferences.h>

Preferences preferences;
bool wifiConnected = false;

//PC IP address
bool foundServer = false;
String server_ip;
int server_port;
uint32_t dt;

WiFiUDP udp;
uint8_t sendBuffer[1024];

Adafruit_NeoPixel LED(NB_LED, PIN_LED, NEO_GRB + NEO_KHZ800);
void
setRGB(
    int8_t r, int8_t g, int8_t b, int8_t r2 = 0, int8_t g2 = 0, int8_t b2 = 0)
{
    LED.setPixelColor(0, LED.Color(r, g, b));
    LED.setPixelColor(1, LED.Color(r2, g2, b2));
    LED.show();
}
void
blink(int8_t r,
      int8_t g,
      int8_t b,
      int8_t n = 1,
      int dt = 500,
      int8_t r2 = 0,
      int8_t g2 = 0,
      int8_t b2 = 0)
{
    for(int i = 0; i < n; i++)
    {
        setRGB(r, g, b, r2, g2, b2);
        delay(dt);
        setRGB(0, 0, 0);
        delay(dt);
    }
}

void
storeCredentials(String ssid, String password)
{
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
connectToWiFi(String ssid, String password)
{
    WiFi.begin(ssid.c_str(), password.c_str());
    while(WiFi.status() != WL_CONNECTED) { blink(255, 0, 0, 1, 500); }
    blink(0, 255, 0, 2, 500);
    udp.begin(12345);
    wifiConnected = true;
}

void
searchServer()
{
    while(!foundServer)
    {
        blink(0, 255, 255, 1, 500);
        //clear the UDP buffer
        while(udp.parsePacket() > 0);
        udp.beginPacket("255.255.255.255", 12345); // Broadcast address
        uint8_t msg[] = "MONOMOD";
        udp.write(msg, sizeof(msg));
        udp.endPacket();
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
            if(n = 2)
            {
                server_port = (packetBuffer[1] << 8) + packetBuffer[0];
                PRINTLN("Server port: " + String(server_port));
                foundServer = true;
            }
        }
        delay(1000);
    }
}

void
setup()
{
    LED.begin();
    LED.setBrightness(255);
    Serial.begin(115200);

    if(existsCredentials())
    {
        String ssid, password;
        readCredentials(ssid, password);
        connectToWiFi(ssid, password);
    }
}

void
loop()
{
    //Serial communication for the credentials setup
    if(Serial.available() > 3)
    {
        char cmd = Serial.read();
        switch(cmd)
        {
        case 'C':
        {
            String ssid, password;
            parseCredentials(ssid, password);
            storeCredentials(ssid, password);
            connectToWiFi(ssid, password);
        }
        break;
        default:
            //blink orange
            blink(255, 165, 0, 1, 500);
            break;
        }
    }

    //if the wifi is connected
    if(wifiConnected)
    {

        if(!foundServer)
            searchServer();

        //connect to the PC
        WiFiClient client;
        client.connect(server_ip.c_str(), server_port);
        PRINTLN("Connecting to the PC");
        blink(0, 255, 0, 5, 100);
        if(client.connected())
        {
            PRINTLN("Connected to the PC");
            sendBuffer[0] = 'A';
            client.write(sendBuffer, 1);
        }
        while(client.connected())
        {
            if(client.available())
            {
                uint8_t c = client.read();
                switch(c)
                {
                case 'R':
                    blink(255, 0, 0, 1, 500);
                    break;
                case 'G':
                    blink(0, 255, 0, 1, 500);
                    break;
                case 'B':
                    blink(0, 0, 255, 1, 500);
                    break;
                case 'T':
                    dt = micros();
                    client.write((char *)&dt, sizeof(dt));
                    break;
                case 'S':
                {
                    uint8_t nbsample = client.read();
                    int i = 0;
                    while(i < 1024 && i * 2 < nbsample)
                    {
                        ((uint16_t *)sendBuffer + 2)[i] = i * 2;
                        i++;
                    };
                    sendBuffer[0] = 'R';
                    sendBuffer[1] = i;
                    client.write(sendBuffer, i * 2);
                }
                }
            }
        }

        //disconnect from the PC
        client.stop();
        foundServer = false;
        PRINTLN("Disconnected from the PC");

        //blink the LED
        blink(0, 0, 255, 1, 5000);
    }
    else
    {
        //blink red
        blink(255, 0, 0, 1, 100);
    }
}