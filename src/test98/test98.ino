#include <SPI.h>

// 75: DVDD
// 74: DRDY
// 73: GPIO4
// 72: GPIO3
// 71: GPIO2
// 70: DOUT
// 69: GPIO1
// 68: DAISY
// 67: SCLK
// 66: CS
// 65: START
// 64: CLK
// 63: RESET
// 62: PWDN
// 61: DIN
// 60: DGND

//88: CLKSEL
//28: AVDD

#define CS_PIN 2
#define PWDN_PIN 9
#define RESET_PIN 8
#define START_PIN 7
#define DRDY_PIN 6
#define CLKSEL_PIN 5

SPISettings SPIsettings(2000000, MSBFIRST, SPI_MODE1);

void
sendCmd(byte cmd)
{
    SPI.beginTransaction(SPIsettings);
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(cmd);
    delayMicroseconds(1);
    digitalWrite(CS_PIN, HIGH);
    SPI.endTransaction();
}

byte
readReg(byte address)
{
    SPI.beginTransaction(SPIsettings);
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(0x20 | address); // Read command (0x20) | address
    delayMicroseconds(2);
    SPI.transfer(0x00); // Send dummy byte
    delayMicroseconds(2);
    byte value = SPI.transfer(0x00); // Read value
    delayMicroseconds(2);
    digitalWrite(CS_PIN, HIGH);
    SPI.endTransaction();
    return value;
}

void
setup()
{
    Serial.begin(115200);

    pinMode(CS_PIN, OUTPUT);
    pinMode(PWDN_PIN, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);
    pinMode(START_PIN, OUTPUT);
    pinMode(DRDY_PIN, INPUT);
    pinMode(CLKSEL_PIN, OUTPUT);
    

    digitalWrite(PWDN_PIN, HIGH);
    digitalWrite(RESET_PIN, HIGH);
    digitalWrite(CS_PIN, HIGH);
    digitalWrite(START_PIN, LOW);
    digitalWrite(CLKSEL_PIN, HIGH);

    delay(1000);
    while(!Serial);
    delay(3000);
    Serial.println("Initializing ADS1298... ");
    Serial.println(digitalRead(DRDY_PIN));
    //while(digitalRead(DRDY_PIN));

    // Initialize SPI
    SPI.begin();
    sendCmd(0x06);
    delay(1000);

    // Send SDATAC command
    sendCmd(0x11);
    delay(1100);
    for(int i = 0; i < 0x19; i++)
    {
        byte reg = readReg(i);
        Serial.print("Reg ");
        Serial.print(i, HEX);
        Serial.print(": ");
        Serial.print(reg, HEX);
        Serial.print("\t");
        Serial.println(reg, BIN);
        delay(100);
    }
    while(1) { delay(1000); }
}

void
loop()
{
    // Add code here if further communication is needed
}
