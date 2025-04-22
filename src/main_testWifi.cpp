#include "clvHd.hpp"
#include <iomanip>
// #include <iostream>
// #include <stdio.h>
// #include <stdlib.h>

void
usage(char *name)
{
    std::cerr << "Usage: " << name << " <serial_port>" << std::endl;
}

int
main(int argc, char *argv[])
{
    
    try
    {
        std::cout << "CleverHand Wifi Interfacee:" << std::endl;
        ClvHd::Device device(1);
        device.initWifi();
        std::cout << "Device initialized" << std::endl;
    //     while(clvhd.nbClients() < 1)
    // // {
        // // }
        
        ClvHd::EMG_ADS1293Pack emg_pack(&device, 5);
        emg_pack.setup();
        
        ClvHd::EMG_ADS1293Config config;
        emg_pack.configure(config);
        
        emg_pack.start_acquisition();
        std::cout << "EMG modules started" << std::fixed << std::setprecision(3);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        while(true)
        {
            std::vector<ClvHd::Value *> values = emg_pack.read_all();
            double timestamp = values[0]->time_s + values[0]->time_ns / 1000000.0;
            std::cout << "t: " << timestamp << " ";
            for(size_t i = 0; i < emg_pack.modules.size(); i++)
            {
                std::cout << i << ": [";
                for(int j = 0; j < values[i]->data.size(); j++)
                {
                    std::cout << std::setw(6) << std::setfill(' ') << 1000 * values[i]->data[j] << " "; 
                    // values[i]->data[j] * 1000 << " ";
                }
                std::cout << "]\t";
            }
            std::cout << "\xd" << std::flush;
            usleep(50000);
        }

    }
    catch(std::exception &e)
    {
        std::cerr << "[ERROR] Got an exception: " << e.what() << std::endl;
    }
    catch(std::string str)
    {
        std::cerr << "[ERROR] Got an exception: " << str << std::endl;
        usage(argv[0]);
    }

    return 0; // success
}
