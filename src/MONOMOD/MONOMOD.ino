#include "WiFi.h"
#include "WiFiUdp.h"
#include <Adafruit_NeoPixel.h>

#define DEBUG 1
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
Adafruit_NeoPixel LED(NB_LED, PIN_LED, NEO_GRB + NEO_KHZ800);

//connect to your local wi-fi network
const char *ssid = "A&A";
const char *password = "Df8ttktg5chw";

//PC IP address
const char *pc_ip = "192.168.0.15";
const int pc_port = 5000;
bool foundServer = false;
String server_ip;
int server_port;
uint32_t dt;

WiFiUDP udp;

uint8_t sendBuffer[1024];

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
setup()
{
    LED.begin();
    LED.setBrightness(255);

    Serial.begin(115200);

    //connect to your local wi-fi network
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED)
    {
        PRINTLN("Connecting to WiFi..");
        blink(255, 0, 0, 1, 500);
    }

    // Serial.println("Connected to the WiFi network");
    PRINTLN("Connected to the WiFi network at IP: ");
    PRINTLN(WiFi.localIP());

    udp.begin(12345);
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
loop()
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
            Serial.println((int)c);
            switch (c)
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