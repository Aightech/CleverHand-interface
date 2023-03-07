#ifndef CLVHDMASTER_H
#define CLVHDMASTER_H

#include <cmath>
#include <cstring>
#include <iostream> // std::cout, std::endl
#include <string>
#include <vector>

// Linux headers
#include <errno.h>   // Error integer and strerror() function
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#include <stdint.h> // uint8_t, uint16_t, uint32_t, uint64_t
#include <stdio.h>  // Standard input/output definitions

#include "clvHduCtrl.hpp" // com control class
#include "registers.hpp"  // Registers class
#include "strANSIseq.hpp" // ANSI sequences class

#define CLV_HD_EMG_MAX_NUM_OF_CHANNELS 16

namespace ClvHd
{

/**
 * @brief The CleverHand Master board class
 *
 * This class is the main class of the library.
 * It is used to communicate with the master board and to read/write registers of each modules.
 *
 * @author Alexis Devillard
 * @date 2022
 */
class Master : virtual public ESC::CLI
{
    using clk = std::chrono::system_clock;
    using sec = std::chrono::duration<double>;

    public:
    struct Config
    {
        int nbChannels;
        int route_table[CLV_HD_EMG_MAX_NUM_OF_CHANNELS][2];
        bool chx_enable[CLV_HD_EMG_MAX_NUM_OF_CHANNELS];
        int res;
        int freq;
        int gains[8];
        int mode;
    };

    enum class Mode
    {
        START,
        STOP,
        RESET,
        CONFIG,
        RUN,
        IDLE,
        ERROR
    };
    enum class Type
    {
        EMG,
        EEG,
        IMU,
        FSR,
        TEMP,
        PRESS,
        ACC,
        GYRO,
        MAG,
        ECG,
        EDA,
        BATT,
        FES,
        HAPTIC,
        LIGHT,
        SOUND,
        LED,
        BUTTON,
        SWITCH,
        POT,
        NONE
    };

    struct Module
    {
        int id;
        int version;
        std::vector<Type> types;
        Mode mode;
        Config config;
    };

    public:
    Master(int verbose = -1) : ESC::CLI(verbose, "ClvHd-Master"){};
    ~Master();

    int
    config(Config config);

    int
    set_mode(Mode mode);

    int
    get_value(int id, int channel, int precision = 2, bool fetch = true);

    int
    get_version();

    int
    get_nb_modules();

    int
    get_status();

    int
    set_LEDs(int id, int ch, int r, int g, int b);

    operator std::string() const
    {
        return "Master board: " + std::to_string(m_modules.size()) +
               " module(s) connected.";
    };

    private:
    int m_version;
    std::vector<Module> m_modules;
    uCtrl* m_uCtrl;
};
} // namespace ClvHd
#endif
