#include <SPI.h>
#include <Wire.h>

#define READ 0b10000000
#define WRITE 0b00000000
#define REG_MASK 0b01111111

#define SPI_PICO 19  
#define SPI_POCI 16  
#define SPI_SCK 18   


//class used to unify the different functions used to interact with the EMG modules
class ClvHd {
public:
  // Initialise the SPI bus and the selction pins
  ClvHd(){};
  ~ClvHd(){};

  void
  begin() {
    SPI.setRX(SPI_POCI);
    SPI.setSCK(SPI_SCK);
    SPI.setTX(SPI_PICO);
    SPI.begin();
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    for (int i = 0; i < 5; i++) pinMode(m_addPins[i], OUTPUT);
    selectBrd(0xff);
    pinMode(m_availPin, INPUT_PULLUP);
    pinMode(m_clkPin, OUTPUT);  //CLK
    pinMode(m_outPin, OUTPUT);  //pull LOW to stop counting
    digitalWrite(m_outPin, LOW);
    digitalWrite(m_clkPin, LOW);

    this->m_nbModule = 0;
  };

  /**
     * @brief Initialise the modules and return the number of module found.
     *
     * @return uint8_t number of module found
     */
  uint8_t
  initModules() {
    if (initialized)
      return this->m_nbModule;

    int width = 1;
    //send pulse as long as the m_availPin is pulled high by a unaddressed module
    while (digitalRead(m_availPin) == 0) {
      digitalWrite(m_clkPin, HIGH);
      delayMicroseconds(1);
      digitalWrite(m_clkPin, LOW);
      delayMicroseconds(1);
      this->m_nbModule++;
      delay(width);
    }
    //init i2c
    Wire.setSDA(4);  // Pin 6 on Raspberry Pi Pico (GPIO4)
    Wire.setSCL(5);  // Pin 7 on Raspberry Pi Pico (GPIO5)
    Wire.begin();
      initialized = true;
    return this->m_nbModule;
  }

  /**
     * @brief Select the module by activating the coreponding address pins.
     *
     * @param id Address of the module to select (0 to 32)
     */
  void
  selectBrd(uint8_t id) {
    for (unsigned i = 0; i < 5; i++)
      digitalWrite(m_addPins[i], (id >> i) & 1);
  }

  /**
     * @brief Read n uint8_ts starting from the reg address of the module coreponding to id.
     *
     * @param n_cmd Number of uint8_ts to read from the module
     * @param cmd Array of uint8_ts to send to the module
     * @param val Array of uint8_ts to store the read values
     * @param n Number of uint8_ts to read from the module
     * @param id Address of the module to read from (0 to 32)
     */
  void
  readCmd(
    uint8_t n_cmd, uint8_t cmd[], uint8_t n, uint8_t val[], uint8_t id = 15) {
    selectBrd(id);
    for (unsigned i = 0; i < n_cmd; i++) SPI.transfer(cmd[i]);
    for (unsigned i = 0; i < n; i++) *(val + i) = SPI.transfer(0x00);
    selectBrd(0x00);
  }

13:55:57.168 -> 1 Addr: 0 Reg: 33
13:55:57.168 -> 2 Addr: 1 Reg: 33
13:55:57.168 -> 3 Addr: 3 Reg: 33
13:55:57.168 -> 4 Addr: 2 Reg: 33
13:55:57.168 -> 5 Addr: 6 Reg: 33
13:55:57.168 -> 6 Addr: 7 Reg: 33
13:55:57.168 -> 7 Addr: 5 Reg: 33
13:55:57.168 -> 8 Addr: 4 Reg: 33
13:55:57.168 -> 9 Addr: C Reg: 33
13:55:57.168 -> 10 Addr: D Reg: 33
13:55:57.168 -> 11 Addr: F Reg: 33
13:55:57.168 -> 12 Addr: E Reg: 33
13:55:57.168 -> 13 Addr: A Reg: 33
13:55:57.168 -> 14 Addr: B Reg: 33
13:55:57.168 -> 15 Addr: 8 Reg: 33


13:55:58.482 -> 1 Addr: 0 Reg: 33
13:55:58.482 -> 2 Addr: 1 Reg: 33
13:55:58.482 -> 3 Addr: 3 Reg: 33
13:55:58.482 -> 4 Addr: 2 Reg: 33
13:55:58.482 -> 5 Addr: 6 Reg: 33
13:55:58.482 -> 6 Addr: 7 Reg: 33
13:55:58.482 -> 7 Addr: 5 Reg: 33
13:55:58.482 -> 8 Addr: 4 Reg: 33
13:55:58.482 -> 9 Addr: C Reg: 33
13:55:58.482 -> 10 Addr: D Reg: 33
13:55:58.482 -> 11 Addr: F Reg: 33
13:55:58.482 -> 12 Addr: E Reg: 33
13:55:58.482 -> 13 Addr: A Reg: 33
13:55:58.482 -> 14 Addr: B Reg: 33

  /**
     * @brief Write n uint8_t to the reg ad the module coreponding to id.
     *
     * @param n_cmd Number of uint8_ts to write to the module
     * @param cmd Array of uint8_ts to send to the module
     * @param n Number of uint8_ts to write to the module
     * @param val Array of uint8_ts to write to the module
     * @param id Address of the module to write to (0 to 32)
     */
  void
  writeCmd(
    uint8_t n_cmd, uint8_t cmd[], uint8_t n, uint8_t val[], uint8_t id = 15) {
    selectBrd(id);
    for (unsigned i = 0; i < n_cmd; i++) SPI.transfer(cmd[i]);
    for (unsigned i = 0; i < n; i++) SPI.transfer(val[i]);
    selectBrd(0x00);
  }

  uint8_t
  nbModules() {
    return m_nbModule;
  }

private:
  bool initialized = false;
  const int m_addPins[5] = { 2, 3, 4, 5, 6 };
  int m_nbModule = 0;
  int m_availPin = 18;
  int m_clkPin = 19;
  int m_outPin = 8;

  // SPIClass mySPI(spi0); // Use spi0 interface

};
