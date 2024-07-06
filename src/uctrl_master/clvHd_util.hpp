#include <SPI.h>

#define READ 0b10000000
#define WRITE 0b00000000
#define REG_MASK 0b01111111

//class used to unify the different functions used to interact with the EMG modules
class ClvHd
{
    public:
    // Initialise the SPI bus and the selction pins
    ClvHd(){};
    ~ClvHd(){};

    void
    begin()
    {
        SPI.begin();
        SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
        for(int i = 0; i < 5; i++) pinMode(m_addPins[i], OUTPUT);
        selectBrd(0xff);
        

        this->m_nbModule = 1;//initModules();
    };

    /**
     * @brief Initialise the modules and return the number of module found.
     *
     * @return uint8_t number of module found
     */
    uint8_t
    initModules()
    {
        uint8_t nbModule = 0;
        // set output pin to HIGH, then pulse clock pin. at every pulse, a new module will be initialised if present
        // if present, the module will pull the data pin low for a short period of time
        digitalWrite(m_outPin, HIGH);
        pinMode(m_dataPin, INPUT_PULLDOWN);
        //loop until no more module is found ( waiting for the data pin to be pulled low for a short period of time)
        while(1)
        {
            digitalWrite(m_clkPin, HIGH);
            delayMicroseconds(1);
            digitalWrite(m_clkPin, LOW);
            delayMicroseconds(1);
            long time = micros();
            while(digitalRead(m_dataPin) == HIGH)
            {
                if(micros() - time > 1000)
                {
                    return nbModule;
                }
            }
        }
        return nbModule;
    }

    /**
     * @brief Select the module by activating the coreponding address pins.
     *
     * @param id Address of the module to select (0 to 32)
     */
    void
    selectBrd(uint8_t id)
    {
        for(unsigned i = 0; i < 5; i++)
            digitalWrite(m_addPins[i], (id >> i) & 1);
    }

    /**
     * @brief Read n bytes starting from the reg address of the module coreponding to id.
     *
     * @param reg Register address to read from
     * @param val Buffer to store the read values
     * @param n Number of bytes to read
     * @param id Address of the module to read from (0 to 32)
     * @param order Order of the bytes in the buffer (true for MSB first, false for LSB first)
     */
    void
    readRegister(byte reg,
                 byte val[],
                 unsigned n = 1,
                 uint8_t id = 15,
                 bool order = true)
    {
        byte dataToSend = READ | (REG_MASK & reg);

        selectBrd(id);
        SPI.transfer(dataToSend);
        if(order)
            for(unsigned i = 0; i < n; i++) *(val + i) = SPI.transfer(0x00);
        else
            for(unsigned i = n - 1; i >= 0; i--)
                *(val + i) = SPI.transfer(0x00);
        selectBrd(0xff);
    }

    /**
     * @brief Write 1 byte to the reg address of the module coreponding to id.
     *
     * @param reg Register address to write to
     * @param val Value to write
     * @param id Address of the module to write to (0 to 32)
     */
    void
    writeRegister(byte reg, byte val, uint8_t id = 15)
    {
        byte dataToSend = WRITE | (REG_MASK & reg);
        selectBrd(id);
        SPI.transfer(dataToSend);
        SPI.transfer(val);
        selectBrd(0xff);
    }

    uint8_t
    nbModules()
    {
        return m_nbModule;
    }

    private:
    const int m_addPins[5] = {6, 7, 8, 9, 10};
    int m_nbModule = 0;
    int m_clkPin = 3;
    int m_dataPin = 5;
    int m_outPin = 4;
};
