
#include "clvHd.hpp"
#include <lsl_cpp.h>

int
main(int argc, char *argv[])
{

    try
    {
        std::cout << "CleverHand Serial Interfacee:" << std::endl;
        ClvHd::Device device(1);
        device.initSerial("/dev/ttyACM0", 500000, O_RDWR | O_NOCTTY);
        // device.initWifi();

        ClvHd::EMG_ADS1293Pack emg_pack(&device, 3);
        emg_pack.setup();

        ClvHd::EMG_ADS1293Config config;
        config.route_table[0][0] = 4;
        config.route_table[0][1] = 5;
        config.route_table[1][0] = 0;
        config.route_table[1][1] = 0;
        config.route_table[2][0] = 0;
        config.route_table[2][1] = 0;
        config.chx_enable[0] = true;
        config.chx_enable[1] = false;
        config.chx_enable[2] = false;
        emg_pack.configure(config);

        int nb_ch = emg_pack.modules.size() * 3;
        lsl::stream_info info_sample("EMG", "sample_fast", nb_ch, 1000,
                                     lsl::cf_double64);
        lsl::stream_outlet outlet_sample(info_sample);
        std::vector<double> sample_fast(nb_ch);

        emg_pack.start_acquisition();
        // emg_pack.start_streaming();
        std::cout << "EMG modules started" << std::endl;

        std::cout << "[INFOS] Now sending data... " << std::endl;
        while(true)
        {
            std::vector<ClvHd::Value *> values = emg_pack.read_all();
            for(size_t i = 0; i < emg_pack.modules.size(); i++)
            {
                for(int j = 0; j < values[i]->data.size(); j++)
                {
                    sample_fast[3 * i + j] = values[i]->data[j] * 1000;
                }
            }
            double timestamp = values[0]->time_s + values[0]->time_ns / 1000000.0;
            std::cout << "timestamp: " << timestamp << "\t" << sample_fast[0]
                      << "     \xd" << std::flush;
            outlet_sample.push_sample(sample_fast, timestamp);
            usleep(1000);
        }
    }
    catch(std::exception &e)
    {
        std::cerr << "[ERROR] Got an exception: " << e.what() << std::endl;
    }
    catch(std::string str)
    {
        std::cerr << "[ERROR] Got an exception: " << str << std::endl;
        // usage(argv[0]);
    }

    return 0; // success
}
