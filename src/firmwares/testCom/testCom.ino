// esp32_client.ino
// Upload in Arduino IDE (select ESP32‑C6 board)
// Fill in SSID/PASS and PC_IP before use.
#include <WiFi.h>

const char *SSID = "";
const char *PASS = "";
IPAddress PC_IP(192, 168, 0, 15); // your PC’s address
const uint16_t PC_PORT = 5000;

WiFiClient client;

void
setup()
{
    Serial.begin(115200);
    WiFi.begin(SSID, PASS);
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
    WiFi.setSleep(false); // disable modem sleep
}

void
loop()
{
    if(!client.connected())
    {
        if(client.connect(PC_IP, PC_PORT))
        {
            client.setNoDelay(true); // disable Nagle
            Serial.println("Connected");
        }
        else
        {
            delay(500);
            return;
        }
    }
    if(client.available())
    {
        uint32_t timePC = 0;
        client.read((uint8_t *)&timePC, sizeof(timePC));
        timePC = ntohl(timePC);

        uint32_t ts = micros();
        uint32_t ts_net = htonl(ts);
        client.write((uint8_t *)&ts_net, sizeof(ts_net));
        Serial.print("Time: ");
        Serial.print(ts);
        Serial.print(" Round-trip: ");
        Serial.println(ts - timePC);
    }
    delay(10); // send once per second
}
