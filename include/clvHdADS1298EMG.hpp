#ifndef CLV_HD_ADS1298EMG_H
#define CLV_HD_ADS1298EMG_H

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "clvHdController.hpp"
#include "registers.hpp"
#include "strANSIseq.hpp"
#include <stdint.h> // uint8_t, uint16_t, uint32_t, uint64_t

#include "clvHdDevice.hpp"

namespace ClvHd
{

using namespace std;

/**
 * @brief The EMG class
 *
 * This class is used to read EMG data from the ClvHd board.
 */
class EMG_ADS1298 : public Module, virtual public ESC::CLI
{
    public:
    EMG_ADS1298(Controller *controller, int id, int verbose = -1)
        : ESC::CLI(verbose, "EMG_ADS1298")
    {
        // m_precise_value[0] = new int32_t[ADS1298_NB_CH];
        // m_precise_value[1] = new int32_t[ADS1298_NB_CH];
        // m_precise_value[2] = new int32_t[ADS1298_NB_CH];
        // m_fast_adc_max = 0;
        // m_precise_adc_max[0] = 0;
        // m_precise_adc_max[1] = 0;
        // m_precise_adc_max[2] = 0;
        m_id = id;
        m_controller = controller;
    };
    ~EMG_ADS1298();

    uint8_t
    SDATAC()
    {
        uint8_t cmd = 0x11;
        return m_controller->writeCmd(this->id, 1, &cmd);
    }

    uint8_t
    RREG(uint8_t reg, uint8_t n, uint8_t *val)
    {
        uint8_t cmd[2]={(uint8_t)(0b001 | reg), (uint8_t)(n-1)};
        return m_controller->readCmd(this->id, 2, cmd, n, val);
    }

    uint8_t
    readReg(uint8_t reg)
    {
        uint8_t val;
        readReg(reg, 1, &val);
        return val;
    }
    int
    readReg(uint8_t reg, int n, uint8_t *val)
    {
        uint8_t cmd =
            reg | 0b10000000; //set the MSB to 1 to indicate a read operation
        return m_controller->readCmd(this->id, 1, &cmd, n, val);
    }
    int
    writeReg(uint8_t reg, uint8_t val)
    {
        return writeReg(reg, 1, &val);
    }
    int
    writeReg(uint8_t reg, int n, uint8_t *val)
    {
        uint8_t cmd =
            reg & 0b01111111; //set the MSB to 0 to indicate a write operation
        return m_controller->writeCmd(this->id, 1, &cmd, n, val);
    }

    
    // /**
    //  * @brief setup Create and setup the EMG modules in the device.
    //  *
    //  * @param device The device to setup.
    //  * @param chx_enable Select which INA channels are enabled.
    //  * @param route_table Routing configuration of the EMG channels.
    //  * @param chx_high_res Select if the INA channels are high resolution.
    //  * @param chx_high_freq Select if the INA channels are high frequency.
    //  * @param R1 Gain R1 of the INA channels.
    //  * @param R2 Gain R2 of the INA channels.
    //  * @param R3 Gain R3 of the INA channels.
    //  * @param verbose The verbosity level.
    //  */
    // static int
    // setup(Device &device,
    //       bool chx_enable[3] = nullptr,
    //       int route_table[3][2] = nullptr,
    //       bool chx_high_res[3] = nullptr,
    //       bool chx_high_freq[3] = nullptr,
    //       int R1[3] = nullptr,
    //       int R2 = -1,
    //       int R3[3] = nullptr)
    // {
    //     uint32_t mask = 1;
    //     modules_mask = 0; //reset the mask
    //     s_cli.set_verbose(device.controller.get_verbose());
    //     //for each module read the registers REG_ID
    //     s_cli.logln("Scanning the " + std::to_string(device.nb_modules) +
    //                     " modules: ",
    //                 true);
    //     for(int i = 0; i < device.nb_modules; i++)
    //     {
    //         // std::cout << "[ADS1293_test] module " << i << ": ";
    //         s_cli.log("module " + std::to_string(i) + ": ", true);
    //         if(device.modules[i] == nullptr)
    //         {
    //             uint8_t rev_id = 0;
    //             //stop continuous data acquisition
    //             uint8_t cmd = 0x11;
    //             device.controller.writeCmd(i, 1, &cmd);
    //             //read the register REG_ID
    //             uint8_t reg_id = 0;
    //             uint8_t nb=1;
    //             uint8_t cmd_arr[2];
    //             cmd_arr[0] = 0b00100000|0x00;
    //             cmd_arr[1] = nb-1;
    //             usleep(1000000);
    //             int n = device.controller.readCmd(i, 2, cmd_arr, nb, &rev_id);
    //             printf("hex %x\n", rev_id);
    //             if(rev_id == 0x01) // ADS1293
    //             {
    //                 s_cli.log(ESC::fstr("OK\n", {ESC::FG_GREEN, ESC::BOLD}));
    //                 //set the i-th bit to 1 to indicate that the module has the class type
    //                 modules_mask |= mask;
    //                 nb_modules++;
    //                 // EMG_ADS1293 *emg =
    //                 //     new EMG_ADS1293(&device.controller, i, verbose);
    //                 // device.modules[i] = emg;
    //                 // emg->setup(route_table, chx_enable, chx_high_res,
    //                 //            chx_high_freq, R1, R2, R3);
    //             }
    //             else
    //             {
    //                 s_cli.log(ESC::fstr("NO\n", {ESC::FG_RED, ESC::BOLD}));
    //             }
    //         }
    //         else
    //         {
    //             s_cli.log(device.modules[i]->get_type() + " already created\n");
    //         }
    //         mask <<= 1;
    //     }
    //     return nb_modules;
    // };

   

    // each bit represent if the module has the class type
    static uint32_t modules_mask;
    static uint8_t nb_modules;

    private:
    uint8_t m_regs[0x50];

    //ESC::CLI for static functions
    static ESC::CLI s_cli;

    int32_t *m_precise_value[3];
    int32_t m_fast_adc_max;
    int32_t m_precise_adc_max[3];
};

} // namespace ClvHd
#endif //CLV_HD_EMG_H
