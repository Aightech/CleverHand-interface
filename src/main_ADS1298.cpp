// #include "clvHd.hpp"
// #include <iomanip>
// #include <iostream>
// #include <stdio.h>
// #include <stdlib.h>

int
main(int argc, char *argv[])
{
    // std::cout << "CleverHand Serial Interface:" << std::endl;
    // try
    // {
    //     ClvHd::Device device(3);

    //     //open the serial connection between the computer and the controller board
    //     device.controller.open_connection("/dev/ttyACM0", 500000,
    //                                       O_RDWR | O_NOCTTY);
    //     usleep(500000);

    //     //setup the device (count and find the type of modules attached)
    //     int n = device.setup();

    //     //setup the emg modules
    //     bool chx_enable[3] = {true,   // Enable channel 1
    //                           false,  // Disable channel 2
    //                           false}; // Disable channel 3
    //     int route_table[3][2] = {
    //         {1, 2},  // (-) and (+) electrodes of the first channel
    //         {0, 1},  // (-) and (+) electrodes of the second channel
    //         {0, 1}}; // (-) and (+) electrodes of the third channel
    //     bool chx_high_res[3] = {
    //         false, true, true}; // Enable or disable the high resolution mode
    //     bool chx_high_freq[3] = {
    //         false, true, true}; // Enable or disable the high frequency mode
    //     int R1[3] = {2, 4, 4};  // Gain R1 of the INA channels
    //     int R2 = 4;             // Gain R2 of the INA channels
    //     int R3[3] = {4, 4, 4};  // Gain R3 of the INA channels
    //     // Create and setup the EMG modules in the device
    //     int nb_EMG_module =
    //         ClvHd::EMG_ADS1298::setup(device, chx_enable, route_table,
    //                                   chx_high_res, chx_high_freq, R1, R2, R3);
    //     return 0;

    //     //return 0;

    //     //start the emg modules
    //     ClvHd::EMG_ADS1293::start_acquisition(device);

    //     std::vector<double> sample_fast(nb_EMG_module * 3);
    //     std::vector<double> sample_precise(nb_EMG_module * 3);
    //     uint8_t flags[nb_EMG_module];
    //     for(int t = 0;; t++)
    //     {
    //         uint64_t timestamp = ClvHd::EMG_ADS1293::read_all(
    //             device, sample_fast.data(), sample_precise.data(), flags);
    //         std::cout << "timestamp: " << timestamp << " ";
    //         for(int i = 0; i < nb_EMG_module; i++)
    //         {
    //             for(int j = 0; j < 3; j++)
    //             {
    //                 //check if the value is available
    //                 if((flags[i] >> (2 + j)) & 0b1) // 5+j for precise value
    //                     std::cout << std::fixed << std::setprecision(2)
    //                               << sample_fast[3 * i + j] * 1000 << " ";
    //                 else // add NaN
    //                     std::cout << "NaN ";
    //             }
    //             std::cout << std::fixed << std::setprecision(2)
    //                       << sample_fast[i] * 1000 << " ";
    //         }
    //         std::cout << std::endl;
    //         usleep(500000);
    //     }
    // }
    // catch(std::exception &e)
    // {
    //     std::cerr << "[ERROR] Got an exception: " << e.what() << std::endl;
    // }
    // catch(std::string str)
    // {
    //     std::cerr << "[ERROR] Got an exception: " << str << std::endl;
    // }

    return 0; // success
}
