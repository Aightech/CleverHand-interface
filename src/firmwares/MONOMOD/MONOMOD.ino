#include "WiFi.h"
#include "WiFiUdp.h"
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
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
#define SPI_CS 7
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
uint8_t recvBuffer[1024];
uint8_t spiCmd[255];

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
    blink(0, 255, 0, 5, 100);
}

void
connectToWiFi(String ssid, String password, int timeout = 4000)
{
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long start = millis();
    while(WiFi.status() != WL_CONNECTED && millis() - start < timeout)
        blink(255, 165, 0, 1, 500);//blink orange
    if(WiFi.status() == WL_CONNECTED)
    {
        PRINTLN("Connected to the WiFi network");
        wifiConnected = true;
        udp.begin(12345);
    }
    else
    {
        PRINTLN("Failed to connect to the WiFi network");
        wifiConnected = false;
    }
}

void
searchServer()
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
}

void
setup()
{
    LED.begin();
    LED.setBrightness(255);
    Serial.begin(115200);

    SPI.begin();
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    pinMode(SPI_CS, OUTPUT);
    digitalWrite(SPI_CS, HIGH);

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
    if(wifiConnected && !foundServer)
        searchServer();
    if(wifiConnected && foundServer)
    {
        //connect to the PC
        WiFiClient client;
        client.connect(server_ip.c_str(), server_port);
        PRINTLN("Connecting to the PC");
        blink(0, 255, 0, 5, 100);
        if(client.connected())
        {
            PRINTLN("Connected to the PC");
            String macAddress = WiFi.macAddress();
            sendBuffer[0] = 'A';
            sendBuffer[1] = macAddress.length();
            //copy the mac address
            for(int i = 0; i < macAddress.length(); i++)
                sendBuffer[i + 2] = macAddress[i];
            client.write(sendBuffer, macAddress.length() + 2);
        }
        while(client.connected())
        {
            if(client.available())
            {
                uint8_t c = client.read();
                switch(c)
                {
                case 'l': //set leds
                {
                    uint8_t rgbbuff[6];
                    client.readBytes(rgbbuff, 6);
                    setRGB(rgbbuff[0], rgbbuff[1], rgbbuff[2], rgbbuff[3],
                           rgbbuff[4], rgbbuff[5]);
                }
                case 'r': //read spi
                {
                    uint8_t nr = client.read(); //number of bytes to read
                    uint8_t nc = client.read(); //size of the command to send
                    client.readBytes(spiCmd, nc);
                    digitalWrite(SPI_CS, LOW);
                    //send the command
                    for(int i = 0; i < nc; i++) SPI.transfer(spiCmd[i]);
                    //read the data
                    for(int i = 0; i < nr; i++) sendBuffer[i] = SPI.transfer(0);
                    digitalWrite(SPI_CS, HIGH);
                    client.write(sendBuffer, nr);
                }
                case 'w': //write spi
                {
                    uint8_t nc = client.read();   //size of the command to send
                    client.readBytes(spiCmd, nc); //read the command
                    digitalWrite(SPI_CS, LOW);
                    for(int i = 0; i < nc; i++) SPI.transfer(spiCmd[i]);
                    digitalWrite(SPI_CS, HIGH);
                }
                default:
                {
                    //unknown cmd, clear client buffer
                    while(client.available()){client.read();}
                }

                }
            }
            delay(1);
        }
        blink(255, 165, 0, 10, 100);//blink orange
        //disconnect from the PC
        client.stop();
        foundServer = false;
        PRINTLN("Disconnected from the PC");

        //blink the LED
        blink(0, 0, 255, 1, 500);
    }
    else
    {
        //blink red
        blink(255, 0, 0, 1, 100);
    }
}