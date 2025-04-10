
#include "clvHd.hpp"
#include <lsl_cpp.h>

void
usage(char *name)
{
    std::cerr << "Usage: " << name << " <serial_port>" << std::endl;
}

int
main(int argc, char *argv[])
{
    std::string port;
    //default port
#if defined(__linux__)
    port = "/dev/ttyACM0";
#elif defined(__APPLE__)
    port = "/dev/tty.usbmodem145149601";
#elif defined(_WIN32)
    port = "COM3";
#endif

    if(argc == 2)
        port = argv[1];

    try
    {
        std::cout << "CleverHand Serial Interfacee:" << std::endl;
        ClvHd::Device device(3);
        std::cout << "Opening serial port: " << port << std::endl;
        device.initSerial(port.c_str());

        ClvHd::EMG_ADS1293Pack emg_pack(&device, 3);
        emg_pack.setup();

        ClvHd::EMG_ADS1293Config config;
        emg_pack.configure(config);

        int nb_ch = emg_pack.modules.size() * 3;
        lsl::stream_info info_sample("EMG", "sample_fast", nb_ch, 0,
                                     lsl::cf_double64);
        lsl::stream_outlet outlet_sample(info_sample);
        std::vector<double> sample_fast(nb_ch);

        emg_pack.start_acquisition();
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

// int
// main(int argc, char *argv[])
// {
// std::cout << "CleverHand Serial Interface:" << std::endl;
// try
// {
//     ClvHd::Device device(3);

//     //open the serial connection between the computer and the controller board
//     device.controller.open_connection("/dev/tty.usbmodem145149601", 500000, O_RDWR | O_NOCTTY);
//     usleep(500000);

//     //setup the controller board (count and find the type of modules attached)
//     device.controller.setup();

//     //setup the emg modules
//     bool chx_enable[3] = {true, false, false}; // Enable or disable the channels
//     int route_table[3][2] = {{0, 1}, {0, 1}, {0, 1}};// Define the electrodes to use for each channel
//     bool chx_high_res[3] = {true, true, true}; // Enable or disable the high resolution mode
//     bool chx_high_freq[3] = {true, true, true}; // Enable or disable the high frequency mode
//     int R1[3] = {4, 4, 4}; // Gain R1 of the INA channels
//     int R2 = 4; // Gain R2 of the INA channels
//     int R3[3] = {4, 4, 4}; // Gain R3 of the INA channels
//     // Create and setup the EMG modules in the device
//     int nb_EMG_module = ClvHd::EMG_ADS1293::setup(device, chx_enable, route_table,  chx_high_res, chx_high_freq, R1, R2, R3);

//     //start the emg modules
//     ClvHd::EMG_ADS1293::start_acquisition(device);

//     int nb_ch = nb_EMG_module * 3;
//     lsl::stream_info info_sample("EMG", "sample_fast", nb_ch, 0,
//                                  lsl::cf_double64);
//     lsl::stream_outlet outlet_sample(info_sample);
//     std::vector<double> sample_fast(nb_ch);
//     std::vector<double> sample_precise(nb_ch);
//     uint8_t flags[nb_EMG_module];
//     std::cout << "[INFOS] Now sending data... " << std::endl;
//     for(int t = 0;; t++)
//     {
//         uint64_t timestamp = ClvHd::EMG_ADS1293::read_all(device, sample_fast.data(), sample_precise.data(), flags);
//         std::cout << "timestamp: " << timestamp << " ";
//         outlet_sample.push_sample(sample_fast, timestamp/1000000.0);
//     }
//     // for(int t = 0;; t++)
//     // {
//     //     uint64_t timestamp;
//     //     master.read_all_signal(&timestamp);
//     //     // std::cout << "timestamp: " << timestamp << " ";
//     //     // std::cout << (int)master.m_EMG[0]->get_regs()[ClvHd::DATA_STATUS_REG] << " 0b" << master.byte2bits(master.m_EMG[0]->get_regs()[ClvHd::DATA_STATUS_REG]) << std::endl;
//     //     for(int i = 0; i < 3; i++)
//     //     {
//     //         sample[i] = master.fast_EMG(0, i);
//     //         sample[i + 3] = master.precise_EMG(0, i);
//     //     }
//     //     outlet_sample.push_sample(sample, timestamp/1000000.0);
//     //     std::cout << "t: " << std::setw(9) << timestamp << ", EMG: " << std::setw(7) << sample[0] << " " << std::setw(7) << sample[1] << " " << std::setw(7) << sample[2] << " " << std::setw(7) << sample[3] << " " << std::setw(7) << sample[4] << " " << std::setw(7) << sample[5] << std::endl;
//     // }
// }
// catch(std::exception &e)
// {
//     std::cerr << "[ERROR] Got an exception: " << e.what() << std::endl;
// }
// catch(std::string str)
// {
//     std::cerr << "[ERROR] Got an exception: " << str << std::endl;
// }

//     return 0; // success
// }
