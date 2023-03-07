#ifndef CLV_HD_EMG_H
#define CLV_HD_EMG_H

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "clvHdMaster.hpp"
#include "registers.hpp"
#include "strANSIseq.hpp"
#include <stdint.h> // uint8_t, uint16_t, uint32_t, uint64_t


namespace ClvHd
{
class Master;

/**
 * @brief The EMG class
 *
 * This class is used to read EMG data from the ClvHd board.
 */
class EMG
{
    public:
x
    public:
    EMG(Master *master, int id);
    ~EMG();

    virtual int
    set_config(Config config);

    virtual double
    get_value(int ch, int precision = 2, bool fetch = true);

    private:
    Master *m_master;
    int m_id;
    bool m_verbose = true;
};

class EMG_V2 : public EMG
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
    EMG_V2(Master *master, int id);
    ~EMG_V2();

    int
    set_config(Config config)
    {
        this->setup(config.route_table, config.chx_enable, config.chx_high_res,
                    config.chx_high_freq, config.R1, config.R2[0], config.R3);
        return 0;
    }

    double
    get_value(int ch, int precision = 2, bool fetch = true)
    {
        if(fetch)
            return (precision == 2) ? this->read_fast_value(ch)
                                    : this->read_precise_value(ch);
        else
            return (precision == 2) ? this->fast_value(ch)
                                    : this->precise_value(ch);
    };

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
    void
    setup(int route_table[3][2],
          bool chx_enable[3],
          bool chx_high_res[3],
          bool chx_high_freq[3],
          int R1[3],
          int R2,
          int R3[3]);

    void
    route_channel(uint8_t channel, uint8_t pos_in, uint8_t neg_in);

    int
    set_mode(Mode mode);
    Mode
    get_mode()
    {
        return m_mode;
    }

    void
    config_clock(bool start, CLK_SRC src, bool en_output);

    void
    enable_channels(bool ch1, bool ch2, bool ch3);

    void
    enable_SDM(bool ch1, bool ch2, bool ch3);

    void
    enable_INA(bool ch1, bool ch2, bool ch3);

    //config a frequency of 1024000Hz or 204800Hz
    void
    config_frequence(bool ch1_freq_double,
                     bool ch2_freq_double,
                     bool ch3_freq_double);
    void
    config_resolution(bool ch1_high_res, bool ch2_high_res, bool ch3_high_res);

    //2 or 4
    void
    config_R1(uint8_t R1_ch1, uint8_t R1_ch2, uint8_t R1_ch3);

    //4, 5, 6 or 8
    void
    config_R2(uint8_t R2);

    //4, 6, 8, 12, 16, 32, 64, 128
    void
    config_R3(int ch, uint8_t R3);

    double
    read_precise_value(int ch);

    double
    read_fast_value(int ch);

    double
    precise_value(int ch);

    double
    fast_value(int ch);

    double
    conv(uint16_t val);

    double
    conv(int ch, int32_t val);

    void
    get_error();

    std::string
    error_range_str();

    std::string
    error_status_str();

    uint8_t *
    get_regs()
    {
        return m_regs;
    };

    private:
    Master *m_master;
    int m_id;
    Mode m_mode;
    uint8_t m_regs[0x50];
    int16_t *m_fast_value;
    int32_t *m_precise_value[3];
    int32_t m_fast_adc_max;
    int32_t m_precise_adc_max[3];
    bool m_verbose = true;
};
} // namespace ClvHd
#endif //CLV_HD_EMG_H
