#define VERSION_MAJOR 3
#define VERSION_MINOR 0

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

#include "clvHd_util.hpp"
#include "stream_utils.hpp"
#include "wifi_utilis.hpp"

uint8_t recv_buff[64];
uint8_t send_buff[1024];
uint64_t *timestamp = (uint64_t *)send_buff;
uint8_t *size_buff = send_buff + 8;
uint8_t *vals_buff = send_buff + 9;
int i, n, reg, nb, id, val, n_cmd;
uint32_t mask_id = 0;

uint32_t stream_mask_id = 0;
uint8_t stream_n = 0;
uint8_t stream_n_cmd = 0;
uint8_t stream_cmd[64];
uint8_t stream_vals[1024];
bool stream_active = false;

WifiUtil wifiUtil;

MyStream myStream;

ClvHd clvHd;

uint32_t t1 = micros(), t2 = 0;
void
setup()
{
    Serial.begin(460800);
    Serial.setTimeout(1000);

    myStream.setWifiUtil(&wifiUtil);

    delay(500);
    clvHd.begin();

    if(wifiUtil.existsCredentials())
    {
        String ssid, password;
        wifiUtil.readCredentials(ssid, password);
        wifiUtil.connectToWiFi(ssid, password);
    }
    myStream.setWifi();
}

void
loop()
{
    if(myStream.available() > 0)
    {
        myStream.readBytes((char *)recv_buff, 1);
        switch(recv_buff[0])
        {
        case 'C':
        {
            String ssid, password;
            wifiUtil.parseCredentials(ssid, password);
            wifiUtil.storeCredentials(ssid, password);
            wifiUtil.connectToWiFi(ssid, password);
        }
        break;
        case 'r': // Reading cmd > 'r' | mask_id | nb_bytes_to_read | n_cmd | cmd[n_cmd] : read n bytes starting from reg
        {
            myStream.readBytes((char *)recv_buff + 1, 6);
            mask_id = *((uint32_t *)(recv_buff + 1)); //4 bytes mask_id
            n = recv_buff[5];     //1 byte number of bytes to read
            n_cmd = recv_buff[6]; //1 byte size of the command
            myStream.readBytes((char *)recv_buff + 7, n_cmd);
            *timestamp = micros(); //8 bytes timestamp stored in send_buff
            int ir = 0;
            for(i = 0; i < clvHd.nbModules(); i++)
            {
                if(mask_id & ((uint32_t)1 << i)) //check if the i-th bit is set
                {
                    //read n bytes starting from reg address of the module i
                    //and store them in vals_buff (send_buff + 9)
                    clvHd.readCmd(n_cmd, recv_buff + 7, n, vals_buff + n * ir,
                                  i + 1);
                    ir++;
                }
            }
            *size_buff = n * ir; //number of bytes read (send_buff + 8)
            PRINTLN("Size: " + String(*size_buff));
            myStream.write(send_buff, 9 + (*size_buff));
            break;
        }
        case 'R': // Streaming cmd > 'r' | mask_id | nb_bytes_to_read | n_cmd | cmd[n_cmd] : read n bytes starting from reg
        {
            myStream.readBytes((char *)recv_buff + 1, 6);
            stream_mask_id = *((uint32_t *)(recv_buff + 1)); //4 bytes mask_id
            stream_n = recv_buff[5]; //1 byte number of bytes to read

            stream_n_cmd = recv_buff[6]; //1 byte size of the command
            myStream.readBytes((char *)stream_cmd, stream_n_cmd);
            if(stream_n > 0)
                stream_active = true;
            else
                stream_active = false;
            break;
        }
        case 'w': //> 'w' | mask id | n | n_cmd | cmd[n_cmd] | val[n] : write n bytes starting from reg
        {
            myStream.readBytes((char *)recv_buff + 1, 6);
            mask_id = *((uint32_t *)(recv_buff + 1)); //4 bytes mask_id
            n = recv_buff[5];     //1 byte number of bytes to write
            n_cmd = recv_buff[6]; //1 byte size of the command
            myStream.readBytes((char *)recv_buff + 7,
                               n_cmd + n); //read n_cmd + n bytes
            *timestamp = micros(); //8 bytes timestamp stored in send_buff
            int iw = 0;
            for(i = 0; i < clvHd.nbModules(); i++)
            {
                if(mask_id & ((uint32_t)1 << i)) //check if the i-th bit is set
                {
                    //write n bytes starting from reg address of the module i
                    clvHd.writeCmd(n_cmd, recv_buff + 7, n,
                                   recv_buff + 7 + n_cmd, i + 1);
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

            myStream.write(send_buff, 9 + *size_buff);
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
            PRINTLN("Init modules: " + String(n));
            *timestamp = 1; //micros();
            *vals_buff = n;
            *size_buff = 1;
            myStream.write(send_buff, 9 + *size_buff);
            break;
        }
        case 'i': // I2c cmd > 'i' | id
        {
            myStream.readBytes((char *)recv_buff + 1, 5);
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
            myStream.readBytes((char *)vals_buff, 3);
            *size_buff = 3;
            myStream.write(send_buff, 9 + *size_buff);
            break;
        }
        case 'v': // Version cmd > 'v'
        {
            *timestamp = micros();
            *(uint8_t *)vals_buff = VERSION_MAJOR;
            *(uint8_t *)(vals_buff + 1) = VERSION_MINOR;
            *size_buff = 2;
            myStream.write(send_buff, 9 + *size_buff);
            break;
        }
        }
    }
    else
    {
        //give time to the wifi to process
        delay(2);
    }

    // if(stream_active)
    // {
    //     //streaming
    //     *timestamp = micros();
    //     int ir = 0;
    //     for(i = 0; i < clvHd.nbModules(); i++)
    //     {
    //         if(stream_mask_id &
    //            ((uint32_t)1 << i)) //check if the i-th bit is set
    //         {
    //             //read stream_n bytes starting from reg address of the module i
    //             //and store them in vals_buff (send_buff + 9)
    //             clvHd.readCmd(stream_n_cmd, stream_cmd, stream_n,
    //                           stream_vals + stream_n * ir, i + 1);
    //             ir++;
    //         }
    //     }
    //     *size_buff = stream_n * ir; //number of bytes read (send_buff + 8)
    //     myStream.write(send_buff, 9 + (*size_buff));
    // }
}
