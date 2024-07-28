#define VERSION_MAJOR 3
#define VERSION_MINOR 0
#include "clvHd_util.hpp"

uint8_t recv_buff[64];
uint8_t send_buff[255];
uint64_t *timestamp = (uint64_t *)send_buff;
uint8_t *size_buff = send_buff + 8;
uint8_t *vals_buff = send_buff + 9;
int i, n, reg, nb, id, val;
uint32_t mask_id = 0;

ClvHd clvHd;

void
setup()
{

    Serial.begin(500000);
    delay(1000);
    clvHd.begin();
}

void
loop()
{
    if(Serial.available() >= 1)
    {
        recv_buff[0] = Serial.read();
        switch(recv_buff[0])
        {
        case 'r': // Reading cmd > 'r' | mask_id | reg | n : read n bytes starting from reg
        {
            Serial.readBytes((char*)recv_buff + 1, 6);
            mask_id = *((uint32_t*)(recv_buff + 1));//4 bytes mask_id
            reg = recv_buff[5]; //1 byte reg
            n = recv_buff[6]; //1 byte number of bytes to read
            *timestamp = micros();//8 bytes timestamp stored in send_buff
            int ir = 0;
            for(i = 0; i < clvHd.nbModules(); i++)
            {
                if(mask_id & ((uint32_t)1 << i))//check if the i-th bit is set
                {
                    //read n bytes starting from reg address of the module i
                    //and store them in vals_buff (send_buff + 9)
                    clvHd.readRegister(reg, vals_buff + n * ir, n, i);
                    ir++;
                }
            }
            *size_buff = n * ir;//number of bytes read (send_buff + 8)
            Serial.write(send_buff, 9+(*size_buff));
            break;
        }
        case 'w': //> 'w' | mask id | reg | n | val : write n bytes starting from reg
        {
            Serial.readBytes((char*)recv_buff + 1, 6);
            mask_id = *((uint32_t*)(recv_buff + 1));//4 bytes mask_id
            reg = recv_buff[5]; //1 byte reg
            n = recv_buff[6]; //1 byte number of bytes to write
            Serial.readBytes((char*)recv_buff + 7, n);
            *timestamp = micros();//8 bytes timestamp stored in send_buff
            int iw = 0;
            for(i = 0; i < clvHd.nbModules(); i++)
            {
                if(mask_id & ((uint32_t)1 << i))//check if the i-th bit is set
                {
                    //write n bytes starting from reg address of the module i
                    clvHd.writeRegister(reg, recv_buff[7 + iw], i);
                    iw++;
                }
            }
            break;
        }
        case 'n': // Nb module cmd > 'n' 
        {
            nb = clvHd.nbModules();
            *timestamp = micros();
            *vals_buff = nb;
            *size_buff = 1;

            Serial.write(send_buff, 9+*size_buff);
            break;
        }
        case 'b': // Blink cmd > 'b' | id | time_cs | nb_repeat
        {
            break;
        }
        case 's': // Set pin cmd > 's' | id | pin | state
        {
            //init clvHd
            uint8_t n = clvHd.initModules();
            *timestamp = 0;//micros();
            *vals_buff = n;
            *size_buff = 1;
            Serial.write(send_buff, 9+*size_buff);
            break;
        }
        case 'i': // I2c cmd > 'i' | id
        {
            Serial.readBytes((char*)recv_buff + 1, 5);
            //i2c communication
            uint8_t id = recv_buff[1];
            Wire.beginTransmission(id);
            Wire.write(recv_buff[2]);
            Wire.write(recv_buff[3]);
            Wire.write(recv_buff[4]);
            Wire.write(recv_buff[5]);
            Wire.endTransmission();
            break;
        }
        case 'm': // Mirror cmd > 'm' | b1 | b2 | b3
        {
            *timestamp = micros();
            vals_buff[0] = recv_buff[2];
            vals_buff[1] = recv_buff[3];
            vals_buff[2] = recv_buff[4];
            *size_buff = 3;
            Serial.write(send_buff, 9+*size_buff);
            break;
        }
        case 'v': // Version cmd > 'v' 
        {
            *timestamp = micros();
            *(uint8_t *)vals_buff = VERSION_MAJOR;
			*(uint8_t *)(vals_buff + 1) = VERSION_MINOR;
            *size_buff = 2;
            Serial.write(send_buff, 9+*size_buff);
            break;
        }
        }
    }
}
