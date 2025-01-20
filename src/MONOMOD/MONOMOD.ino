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

WiFiUDP udp;

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
loop()
{
    bool foundServer = false;
    std::string server_ip;
    int server_port;
    while(!foundServer)
    {
        blink(0, 255, 0, 1, 500);
        udp.beginPacket("255.255.255.255", 12345); // Broadcast address
        uint8_t msg[] = "MONOMOD";
        udp.write(msg, sizeof(msg));
        udp.endPacket();

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
    }
    //connect to the PC
    WiFiClient client;
    while(!client.connect(server_ip.c_str(), server_port))
    {
        PRINTLN("Connecting to PC..");
        blink(255, 0, 255, 1, 500);
    }

    // Serial.println("Connected to the PC");
    PRINTLN("Connected to the PC");
    int32_t dt = micros();
    for(int i = 0; i < 100; i++)
    {
        //send a message to the PC
        dt = i;//micros();
        client.write((char *)&dt, sizeof(dt));
        PRINTLN("Sent: " + String(dt));
    }

    //read the response from the PC
    while(client.available() == 0) delay(1);
    String response = client.readStringUntil('\r');
    PRINTLN(response);

    //disconnect from the PC
    client.stop();
    PRINTLN("Disconnected from the PC");

    //blink the LED
    blink(0, 0, 255, 1, 5000);
}