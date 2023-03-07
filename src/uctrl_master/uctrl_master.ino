//#define TEENSY_41
//#define IOT33
//#define UDP_MODE
#define VERSION 0x0200
#include "clvHd_util.hpp"
#include "com.hpp"


#define DEBUG(x) Serial.print(x)
#define DEBUGF(x,y) Serial.print(x,y)
#define DEBUGLN(x) Serial.println(x)


byte buff[255];
int pkgSize = 6;
byte vals[255] = {0x00, 0x00, 0x00, 0x00};
int i, nb=0;

ClvHdEMG clvHdEMG;
Com com_interface;

void setup()
{
  
  //Serial.begin(500000);
  while(!Serial){}
  com_interface.begin(500000);//, 5000, 192, 168, 127, 253);
  clvHdEMG.begin();
  
}


void loop()
{
  int nn = com_interface.available();
  if (nn >= pkgSize)
  {
    com_interface.read(buff, pkgSize);
    switch (buff[0])
    {
      case 'r':// Reading cmd > 'r' | id | reg | n : read n bytes starting from reg
        {
          clvHdEMG.readRegister(buff[2], vals, buff[3], buff[1]);
          com_interface.write((uint8_t*)vals, buff[3], true);
          break;
        }
        case 'R':// Reading cmd > 'R' | 0 | reg | n : read n bytes starting from reg for each module 
        {
          for(i=0;i<nb;i++)
              clvHdEMG.readRegister(buff[2], vals+buff[3]*i, buff[3], 0xf-i);
          //memset(((uint8_t*)vals)+3, 0, 4);
          
          //(((uint8_t*)vals)+1)[6]=0x40;

          // (((uint8_t*)vals)+1)[9]=0x00;
          // (((uint8_t*)vals)+1)[10]=0x00;
          // (((uint8_t*)vals)+1)[11]=0x00;

          // (((uint8_t*)vals)+1)[12]=0x00;
          // (((uint8_t*)vals)+1)[13]=0x00;
          // (((uint8_t*)vals)+1)[14]=0x00;
          com_interface.write((uint8_t*)vals, buff[3]*nb, true);
          //delay(5);
          
          break;
        }
      case 'w':// Reading cmd > 'w' | id | reg | val
        {
          clvHdEMG.writeRegister(buff[2], buff[3], buff[1]);
           buff[1] = buff[3];
          com_interface.write((uint8_t*)buff, 2, true);
          break;
        }
      case 'n':// Nb module cmd > 'n' | 0 | 0 | 0
        {
          nb = clvHdEMG.get_nbBoard();
          buff[0] = nb;
          buff[1] = 0;
          com_interface.write((uint8_t*)buff, 2, true);
          break;
        }
      case 'b':// Blink cmd > 'b' | id | time_cs | nb_repeat
        {
          clvHdEMG.blink(buff[1], buff[2], buff[3]);
          break;
        }
      case 'm':// Mirror cmd > 'm' | b1 | b2 | b3
        {
          com_interface.write((uint8_t*)buff, 4, true);
          break;
        }
      case 'v':// Version cmd > 'v' | 0 | 0 | 0
        {
          *(uint16_t *)(buff) = VERSION;
          com_interface.write((uint8_t*)buff, 2, true);
          break;
        }
      case 'z':// Reset cmd > 'z' | 0 | 0 | 0
        {
          break;
        }
    }
  }


}
