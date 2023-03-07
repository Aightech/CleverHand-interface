#include <SPI.h>

#define READ 0b10000000
#define WRITE 0b00000000
#define REG_MASK 0b01111111

//class used to unify the different functions used to interact with the EMG modules
class ClvHdEMG
{
  public:
    // Initialise the SPI bus and the selction pins
    ClvHdEMG() {};
    ~ClvHdEMG() {};

    void begin()
    {
      SPI.begin();
      SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
      for (int i = 0; i < 4; i++)
        pinMode(m_addPins[i], OUTPUT);
      pinMode(m_gpio1Pin, OUTPUT);
      digitalWrite(m_gpio1Pin, HIGH);
      
      //pinMode(12, INPUT);
      //digitalWrite(12, LOW);

      // selectBrd(0);
      // uint8_t val = 0;
      // while(1){
      //     m_nb_board = 0;
      //   byte val = 0;
      //   for (unsigned i = 15; i >0; i--)
      //   {
          
      //     //Serial.print(i);
      //     //Serial.print(" ");
      //     readRegister(0x40, &val, 1, i);
      //     //Serial.println((int)val);
      //     if (val == 0x01)
      //       m_nb_board++;
      //     //selectBrd(i);
      //   }
      //   Serial.println((int)m_nb_board);

      //   delay(1);
      // };

      this->m_nb_board = nb_emg_connected();
      
      //Serial.println("hey");
      //Serial.println(this->m_nb_board);
      
      // int id=14;
      // int dt_cs=5;
      // int n=5;
      // selectBrd(id);
      // for (int i = 0; i < 2*n; i++)
      // {
      //   digitalWrite(m_gpio1Pin, i % 2);
      //   delay(dt_cs*10);
      // }
      // for (int j = 15; j > 15 - 8; j--)
      // {
      //   selectBrd(j);
      //   delay(1000);
      // }  
      
      
      //while(1);
      
//      digitalWrite(m_gpio1Pin, HIGH);
//      for(int i =0;;i++)
//      {
//        selectBrd(13+i%2);
//        delay(200);
//      }
//
      // quick blink on available modules

      digitalWrite(m_gpio1Pin, HIGH);
      for (int j = 15; j > 15 -this->m_nb_board ; j--)
        this->blink(j, 2, 3);
    };

    //Select the module by activating the coreponding address pins.
    void selectBrd(uint8_t id)
    {
      for (unsigned i = 0; i < 4; i++)
        digitalWrite(m_addPins[ i] , (id >> i) & 1 );
    }

    // Read n bytes starting from the reg address of the module coreponding to id.
    // All values are store in the val buff
    void readRegister(byte reg, byte val[], unsigned n = 1, uint8_t id = 15, bool order = true)
    {
      byte dataToSend  = READ | (REG_MASK & reg);

      selectBrd(id);
      SPI.transfer(dataToSend);
      if (order)
        for (unsigned i = 0; i < n; i++)
          *(val + i) = SPI.transfer(0x00);
      else
        for (unsigned i = n - 1; i >= 0; i--)
          *(val + i) = SPI.transfer(0x00);
      selectBrd(0x00);
    }

    // Write 1 byte to the reg address of the module coreponding to id.
    // All values are store in the val buff
    void writeRegister(byte reg, byte val, uint8_t id = 15)
    {
      byte dataToSend = WRITE | (REG_MASK & reg);;
      selectBrd(id);
      SPI.transfer(dataToSend);
      SPI.transfer(val);
      selectBrd(0x00);
    }

    // Scan available modules.
    int nb_emg_connected()
    {
      m_nb_board = 0;
      byte val = 0;
      for (unsigned i = 15; i >0; i--)
      {
        
        //Serial.print(i);
        //Serial.print(" ");
        readRegister(0x40, &val, 1, i);
        //Serial.println((int)val);
        if (val == 0x01)
          m_nb_board++;
        selectBrd(i);
        delay(100);
      }
      return m_nb_board;
    }

    uint8_t get_nbBoard()
    {
      return m_nb_board;
    }

    // Blink the module coresponding to id n times for dt_cs centiseconds
    void blink(int id, int dt_cs, int n)
    {
      selectBrd(id);
      for (int i = 0; i < 2*n; i++)
      {
        digitalWrite(m_gpio1Pin, i % 2);
        delay(dt_cs*10);
      }
      selectBrd(0x0e - m_nb_board);
      digitalWrite(m_gpio1Pin, LOW);
    }


  private:
    const int m_addPins[4] = {7, 8, 9, 10};
    const int m_gpio1Pin = 6;
    const int m_resetPin = 14;
    int m_nb_board = 0;
    uint8_t m_buffer[1024]; 
};
