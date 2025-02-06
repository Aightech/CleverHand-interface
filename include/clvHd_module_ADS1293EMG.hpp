#ifndef CLV_HD_ADS1293EMG_H
#define CLV_HD_ADS1293EMG_H

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "clvHd_controller.hpp"
#include "clvHd_device.hpp"
#include "clvHd_module_ADS1293EMG_registers.hpp"
#include "strANSIseq.hpp"
#include <stdint.h> // uint8_t, uint16_t, uint32_t, uint64_t

namespace ClvHd
{

using namespace std;

/**
 * @brief The EMG class
 *
 * This class is used to read EMG data from the ClvHd board.
 */
class EMG_ADS1293 : public Module
{
    public:
    typedef struct
    {
        uint8_t error_lod;
        uint8_t error_status;
        uint8_t error_range1;
        uint8_t error_range2;
        uint8_t error_range3;
        uint8_t error_sync;
        uint8_t error_misc;
        uint8_t padding;
    } Error;

    enum CLK_SRC
    {
        EXTERN = 1,
        INTERN = 0
    };

    public:
    enum Mode
    {
        START_CONV = 0x1,
        STANDBY = 0x2,
        POWER_DOWN = 0x4
    };

    public:
    EMG_ADS1293(Controller *controller, int id, int verbose = -1);
    ~EMG_ADS1293();

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

    /**
     * @brief setup Write all setup parameters to the board.
     *
     * @param route_table Routing configuration of the EMG channels.
     * @param chx_enable Select which INA channels are enabled.
     * @param chx_high_res Select if the INA channels are high resolution.
     * @param chx_high_freq Select if the INA channels are high frequency.
     * @param R1 Gain R1 of the INA channels.
     * @param R2 Gain R2 of the INA channels.
     * @param R3 Gain R3 of the INA channels.
     */
    int
    setup(int route_table[3][2],
          bool chx_enable[3],
          bool chx_high_res[3],
          bool chx_high_freq[3],
          int R1[3],
          int R2,
          int R3[3],
          bool clock_intern = true);

    int
    route_channel(uint8_t channel, uint8_t pos_in, uint8_t neg_in);

    int
    route_channel_test(uint8_t channel, bool pos_test, bool neg_test);

    int
    route_vbat(bool ch1, bool ch2, bool ch3);

    int
    get_route_neg(int ch);
    int
    get_route_pos(int ch);

    int
    set_mode(Mode mode);
    Mode
    get_mode();

    int
    config_clock(bool start, CLK_SRC src, bool en_output);

    bool
    is_clock_started();
    bool
    is_clock_ext();
    bool
    is_clock_output_enabled();

    int
    enable_ADC(bool ch1, bool ch2, bool ch3);
    bool
    is_ADC_enabled(int ch);

    int
    enable_SDM(bool ch1, bool ch2, bool ch3);
    bool
    is_SDM_enabled(int ch);

    int
    enable_INA(bool ch1, bool ch2, bool ch3);
    bool
    is_INA_enabled(int ch);

    //config a frequency of 1024000Hz or 204800Hz
    int
    config_frequence(bool ch1_freq_double,
                     bool ch2_freq_double,
                     bool ch3_freq_double);
    bool
    is_high_freq_enabled(int ch);

    int
    config_resolution(bool ch1_high_res, bool ch2_high_res, bool ch3_high_res);
    bool
    is_high_res_enabled(int ch);

    void
    set_filters(int R1[3], int R2, int R3[3]);
    void
    get_filters(int R1[3], int *R2, int R3[3]);

    void
    update_adc_max();

    double
    read_precise_value(int ch, bool converted = true);

    double
    read_fast_value(int ch, bool converted = true);

    double
    precise_value(int ch, bool converted = true);

    double
    fast_value(int ch, bool converted = true);

    double
    conv(uint16_t val);

    double
    conv(int ch, int32_t val);

    int
    get_error();

    std::string
    error_range_str();

    std::string
    error_status_str();

    std::string
    dump_regs(bool pull = false);

    uint8_t *
    get_regs()
    {
        return m_regs;
    };
    int16_t *m_fast_value;

    static uint64_t
    read_all(Device &device,
             double *fast,
             double *precise,
             uint8_t *flags = nullptr)
    {
        (void)device;
        (void)fast;
        (void)precise;
        (void)flags;
        return 0;
        // uint64_t timestamp = 0;
        // // uint8_t buffer[16 * EMG_ADS1293::nb_modules];
        // uint8_t *buffer = new uint8_t[16 * EMG_ADS1293::nb_modules];

        // uint8_t cmd = ADS1293_Reg::DATA_STATUS_REG | 0b10000000;
        // int n = device.controller.readCmd_multi(modules_mask, 1, &cmd, 16,
        //                                         buffer, &timestamp);
        // if(n != 16 * EMG_ADS1293::nb_modules)
        // {
        //     std::cerr << "Error reading EMG data" << std::endl;
        //     return -1;
        // }

        // for(int i = 0, index = 0; i < device.modules.size(); i++)
        //     if(modules_mask & (((uint32_t)1) << i))
        //     {
        //         // printf("Status register: 0x%02X (0b", buffer[16 * index]);
        //         // for(int j = 0; j < 8; j++)
        //         //     printf("%d", (buffer[16 * index] >> (7 - j)) & 1);
        //         // printf(")\n");
        //         EMG_ADS1293 *emg = EMG_ADS1293::getModule(device, i);
        //         std::copy(buffer + 16 * index,       //index-th status register
        //                   buffer + 16 * (index + 1), //last sample byte
        //                   emg->m_regs + ADS1293_Reg::DATA_STATUS_REG); //dest
        //         for(int ch = 0; ch < 3; ch++)
        //         {
        //             //fast value (2 bytes)
        //             fast[6 * index + ch] = emg->fast_value(ch);
        //             //precise value (3 bytes)
        //             precise[6 * index + ch] = emg->precise_value(ch);
        //         }
        //         if(flags != nullptr)
        //             flags[index] = buffer[16 * index];
        //         index++;
        //     }
        // delete[] buffer;
        // return timestamp;
    };

    bool chx_enable[3];
    int route_table[3][2];
    bool chx_high_res[3];
    bool chx_high_freq[3];
    int R1[3];
    int R2;
    int R3[3];

    // each bit represent if the module has the class type
    static uint32_t modules_mask;
    static uint8_t nb_modules;

    uint8_t* regsAddr() { return m_regs; }

    private:
    uint8_t m_regs[0x50];
    Mode m_mode;
    bool m_adc_enabled[3] = {false, false, false};

    //ESC::CLI for static functions
    static ESC::CLI s_cli;

    int32_t *m_precise_value[3];
    int32_t m_fast_adc_max;
    int32_t m_precise_adc_max[3];
};

class EMG_ADS1293Pack : public ModulePack
{
    public:
    EMG_ADS1293Pack(Device *device, int verbose = -1)
        : ESC::CLI(verbose, "EMG_ADS1293Pack"), ModulePack(device) {

          };
    ~EMG_ADS1293Pack() {};

    void
    setup()
    {
        logln("Setting up EMG_ADS1293Pack", true);
        logln("Scanning the " + std::to_string(m_device->modules.size()) +
                  " modules: ",
              true);

        for(size_t i = 0; i < m_device->modules.size(); i++)
        {
            log("module " + std::to_string(i) + ": ", true);
            if(m_device->modules[i]->typed == false)
            {
                //create a temporary EMG_ADS1293 object to test the module type
                EMG_ADS1293 *emg = new EMG_ADS1293(
                    m_device->controller, m_device->modules[i]->id, m_verbose);
                bool is_correct_type = emg->testType();
                if(is_correct_type) // ADS1293
                {
                    log(ESC::fstr("OK\n", {ESC::FG_GREEN, ESC::BOLD}));
                    this->addModule(emg);
                    //delete and replace the device default module
                    delete m_device->modules[i];
                    m_device->modules[i] = emg;
                }
                else
                {
                    delete emg;
                    log(ESC::fstr("NO\n", {ESC::FG_RED, ESC::BOLD}));
                }
            }
            else
            {
                log(ESC::fstr("ALREADY TYPED\n", {ESC::FG_YELLOW, ESC::BOLD}));
            }
        }
    };

    void
    start_acquisition()
    {
        for(size_t i = 0; i < this->modules.size(); i++)
        {
            logln("start acquisition module " + std::to_string(i), true);
            ((EMG_ADS1293 *)this->modules[i])
                ->set_mode(EMG_ADS1293::START_CONV);
        }
    };

    std::vector<Value *> &
    read_all(bool fast = true)
    {
        uint64_t timestamp = 0;
        uint8_t *buffer = new uint8_t[16 * this->modules.size()];

        uint8_t cmd = ADS1293_Reg::DATA_STATUS_REG | 0b10000000;
        int n = m_device->controller->readCmd_multi(m_mask, 1, &cmd, 16,
                                                buffer, &timestamp);
        if((size_t)n != 16 * this->modules.size())
            throw log_error("Error reading EMG data");

        for(size_t i = 0, index = 0; i < this->modules.size(); i++)
        {
                EMG_ADS1293 *emg = (EMG_ADS1293 *)this->modules[i];
                std::copy(buffer + 16 * index,       //index-th status register
                          buffer + 16 * (index + 1), //last sample byte
                          emg->regsAddr() + ADS1293_Reg::DATA_STATUS_REG); //dest
                for(int ch = 0; ch < 3; ch++)
                {
                    sensorValues[i]->time_s = timestamp/1000000.0;
                    sensorValues[i]->time_ns = timestamp%1000000;
                    if(fast)
                        sensorValues[i]->data[ch] = emg->fast_value(ch);
                    else
                        sensorValues[i]->data[ch] = emg->precise_value(ch);
                }
                index++;
            }
        delete[] buffer;
        return sensorValues;
    };
};

} // namespace ClvHd
#endif //CLV_HD_EMG_H
