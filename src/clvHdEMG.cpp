#include <clvHdEMG.hpp>

namespace ClvHd
{
using namespace ESC;

uint32_t EMG_ADS1293::modules_mask = 0;
uint8_t EMG_ADS1293::nb_modules = 0;

EMG_ADS1293::EMG_ADS1293(Controller *controller, int id, int verbose)
    : 
      ESC::CLI(verbose, "EMG_" + std::to_string(id))
{
    this->id = id;
    this->m_controller = controller;
    logln("Initialised", true);
    for(int i = 0; i < 0x50; i++) m_regs[i] = 0x00;

    m_regs[CONFIG_REG] = 0x02;
    m_regs[LOD_CN_REG] = 0x08;
    m_regs[AFE_PACE_CN_REG] = 0x01;
    m_regs[DIGO_STRENGTH_REG] = 0x03;
    m_regs[R2_RATE_REG] = 0x08;
    m_regs[R3_RATE_CH0_REG] = 0x80;
    m_regs[R3_RATE_CH1_REG] = 0x80;
    m_regs[R3_RATE_CH2_REG] = 0x80;
    m_regs[SYNCB_CN_REG] = 0x40;
    m_regs[RESERVED_0x2D_REG] = 0x09;
    m_regs[ALARM_FILTER_REG] = 0x33;
    m_regs[REVID_REG] = 0x01;

    m_fast_value = (int16_t *)(m_regs + DATA_CH0_PACE_REG);
    for(int i = 0; i < 3; i++)
        m_precise_value[i] = (int32_t *)(m_regs + DATA_CH0_ECG_REG + 3 * i);

    m_fast_adc_max = 0x8000;
    for(int i = 0; i < 3; i++) { m_precise_adc_max[i] = 0x800000; }

    m_fast_value[0] = 0;
    if(id == 0)
        m_fast_value[0] = 0xabcd;
};

EMG_ADS1293::~EMG_ADS1293() { m_controller->writeReg(this->id, CONFIG_REG, 0x02); }

int
EMG_ADS1293::setup(int route_table[3][2],
           bool chx_enable[3],
           bool chx_high_res[3],
           bool chx_high_freq[3],
           int R1[3],
           int R2,
           int R3[3],
              bool clock_intern)
{
    CLK_SRC clk_src = (clock_intern)
                          ? INTERN
                          : EXTERN; //Start clk and output on CLK pin (0x05);

    // standby mode (0x02)
    int n = this->set_mode(STANDBY);
    n += this->config_clock(true, clk_src, true);
    n += this->route_channel(0, route_table[0][0], route_table[0][1]);
    n += this->route_channel(1, route_table[1][0], route_table[1][1]);
    n += this->route_channel(2, route_table[2][0], route_table[2][1]);
    n += this->enable_ADC(chx_enable[0], chx_enable[1], chx_enable[2]);
    n += this->config_resolution(chx_high_res[0], chx_high_res[1],
                                 chx_high_res[2]);
    n += this->config_frequence(chx_high_freq[0], chx_high_freq[1],
                                chx_high_freq[2]);
    n += this->config_R1(R1[0], R1[0], R1[2]);
    n += this->config_R2(R2);
    n += this->config_R3(0, R3[0]);
    n += this->config_R3(1, R3[1]);
    n += this->config_R3(2, R3[2]);

    bool ret = is_clock_ext();
    logln("Seting ", true);
    std::string clk_src_s = std::string(ret ? "extern" : "intern");
    logln("Seting ", true);

    int range[3] = {0, 1, 2};

    logln("Seting up ADS1293 : " + ((n == 26) ? fstr(" OK", {BOLD, FG_GREEN})
                                              : fstr(" ERROR", {BOLD, FG_RED})),
          true);
    logln("Set clock on CLK " + clk_src_s);
    logln("           |   Ch1   |   Ch2   |   Ch3   |");

    log("           |", true);
    for(int i : {0, 1, 2})
        log(std::string(is_ADC_enabled(i) ? " en" : "dis") + "abled |");
    logln("");

    log("Route      |", true);
    for(int i : {0, 1, 2})
        log("(-)" + std::to_string(get_route_pos(i)) + " (+)" +
            std::to_string(get_route_neg(i)) + "|");
    logln("");

    logln("Resolution |  " +
          std::string(is_high_res_enabled(0) ? "high" : " low") + "   |  " +
          std::string(is_high_res_enabled(1) ? "high" : " low") + "   |  " +
          std::string(is_high_res_enabled(2) ? "high" : " low") + "   |");
    logln("Frequence  |  " +
          std::string(is_high_freq_enabled(0) ? "high" : " low") + "   |  " +
          std::string(is_high_freq_enabled(1) ? "high" : " low") + "   |  " +
          std::string(is_high_freq_enabled(2) ? "high" : " low") + "   |");
    log("   R1      |", true);
    for(int i : {0, 1, 2})
    {
        R1[i] = get_R1(i);
        log("   " + std::string(3 - std::to_string(R1[i]).size(), ' ') +
            std::to_string(R1[i]) + "   |");
    }
    logln("");

    R2 = get_R2();
    logln("   R2      |             " +
          std::string(3 - std::to_string(R2).size(), ' ') + std::to_string(R2) +
          "             |");

    log("   R3      |", true);
    for(int i : {0, 1, 2})
    {
        R3[i] = get_R3(i);
        log("   " + std::string(3 - std::to_string(R3[i]).size(), ' ') +
            std::to_string(R3[i]) + "   |");
    }
    logln("");
    return n;
}

int
EMG_ADS1293::route_channel(uint8_t channel, uint8_t pos_in, uint8_t neg_in)
{
    pos_in = (pos_in < 0) ? 0 : (pos_in > 6) ? 6 : pos_in;
    neg_in = (neg_in < 0) ? 0 : (neg_in > 6) ? 6 : neg_in;

    channel = ((channel < 0) ? 0 : (channel > 2) ? 2 : channel);
    uint8_t val = pos_in | (neg_in << 3);
    m_regs[FLEX_CH0_CN_REG + channel] = val;
    return m_controller->writeReg(this->id, FLEX_CH0_CN_REG + channel, val);
}

int
EMG_ADS1293::get_route_neg(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, FLEX_CH0_CN_REG + ch, 1, &val);
    return ((val & 0b111000) >> 3);
}

int
EMG_ADS1293::get_route_pos(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, FLEX_CH0_CN_REG + ch, 1, &val);
    return (val & 0b111);
}

int
EMG_ADS1293::set_mode(Mode mode)
{
    m_mode = mode;
    m_regs[CONFIG_REG] = mode;
    return m_controller->writeReg(this->id, CONFIG_REG, mode);
}

EMG_ADS1293::Mode
EMG_ADS1293::get_mode()
{
    uint8_t val = 0;
    m_controller->readReg(this->id, CONFIG_REG, 1, &val);
    return (Mode)(val);
}

int
EMG_ADS1293::config_clock(bool start, CLK_SRC src, bool en_output)
{
    uint8_t val = (start ? 0x4 : 0x0) | (src << 1) | (en_output ? 0x1 : 0x0);
    m_regs[OSC_CN_REG] = val;
    return m_controller->writeReg(this->id, OSC_CN_REG, val);
}

bool
EMG_ADS1293::is_clock_started()
{
    uint8_t val = 0;
    m_controller->readReg(this->id, OSC_CN_REG, 1, &val);
    return (val >> 2) & 0b1;
}

bool
EMG_ADS1293::is_clock_ext()
{
    uint8_t val = 0;
    m_controller->readReg(this->id, OSC_CN_REG, 1, &val);
    bool ret = (val >> 1) & 0b1;
    //logln("is_clock_ext: " + std::to_string(ret));
    return ret;
}

bool
EMG_ADS1293::is_clock_output_enabled()
{
    uint8_t val = 0;
    m_controller->readReg(this->id, OSC_CN_REG, 1, &val);
    return (val & 0b1);
}

int
EMG_ADS1293::enable_ADC(bool ch0, bool ch1, bool ch2)
{
    uint8_t val =
        (ch0 ? 0 : 0b001001) | (ch1 ? 0 : 0b010010) | (ch2 ? 0 : 0b100100);
    m_regs[AFE_SHDN_CN_REG] = val;
    return m_controller->writeReg(this->id, AFE_SHDN_CN_REG, val);
}

bool
EMG_ADS1293::is_ADC_enabled(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, AFE_SHDN_CN_REG, 1, &val);
    uint8_t mask = 0b001001;
    return !(val & (mask << ch));
}

int
EMG_ADS1293::enable_SDM(bool ch0, bool ch1, bool ch2)
{
    uint8_t val = (m_regs[AFE_SHDN_CN_REG] & 0b111) | (ch0 ? 0 : 0b1000) |
                  (ch1 ? 0 : 0b10000) | (ch2 ? 0 : 0b100000);
    m_regs[AFE_SHDN_CN_REG] = val;
    return m_controller->writeReg(this->id, AFE_SHDN_CN_REG, val);
}

bool
EMG_ADS1293::is_SDM_enabled(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, AFE_SHDN_CN_REG, 1, &val);
    return !((val >> (3 + ch)) & 0b1);
}

int
EMG_ADS1293::enable_INA(bool ch0, bool ch1, bool ch2)
{
    uint8_t val = (m_regs[AFE_SHDN_CN_REG] & 0b111000) | (ch0 ? 0 : 0b1) |
                  (ch1 ? 0 : 0b10) | (ch2 ? 0 : 0b100);
    m_regs[AFE_SHDN_CN_REG] = val;
    return m_controller->writeReg(this->id, AFE_SHDN_CN_REG, val);
}

bool
EMG_ADS1293::is_INA_enabled(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, AFE_SHDN_CN_REG, 1, &val);
    return !((val >> (ch)) & 0b1);
}

int
EMG_ADS1293::config_resolution(bool ch0_high_res, bool ch1_high_res, bool ch2_high_res)
{
    uint8_t val = (m_regs[AFE_RES_REG] & 0b00111000) |
                  (ch0_high_res ? 0b1 : 0) | (ch2_high_res ? 0b010 : 0) |
                  (ch2_high_res ? 0b100 : 0);
    m_regs[AFE_RES_REG] = val;
    return m_controller->writeReg(this->id, AFE_RES_REG, val);
}

bool
EMG_ADS1293::is_high_res_enabled(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, AFE_RES_REG, 1, &val);
    return ((val >> (ch)) & 0b1);
}

int
EMG_ADS1293::config_frequence(bool ch0_freq_double,
                      bool ch1_freq_double,
                      bool ch2_freq_double)
{
    uint8_t val =
        (m_regs[AFE_RES_REG] & 0b111) | (ch0_freq_double ? 0b1000 : 0) |
        (ch1_freq_double ? 0b010000 : 0) | (ch2_freq_double ? 0b100000 : 0);
    m_regs[AFE_RES_REG] = val;
    return m_controller->writeReg(this->id, AFE_RES_REG, val);
}

bool
EMG_ADS1293::is_high_freq_enabled(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, AFE_RES_REG, 1, &val);
    return ((val >> (ch + 3)) & 0b1);
}

int
EMG_ADS1293::config_R1(uint8_t R1_ch0, uint8_t R1_ch1, uint8_t R1_ch2)
{
    R1_ch0 = (R1_ch0 < 3) ? 0b001 : 0; //2 or 4
    R1_ch1 = (R1_ch1 < 3) ? 0b010 : 0; //2 or 4
    R1_ch2 = (R1_ch2 < 3) ? 0b100 : 0; //2 or 4
    uint8_t val = R1_ch0 | R1_ch1 | R1_ch2;
    m_regs[R1_RATE_REG] = val;
    return m_controller->writeReg(this->id, R1_RATE_REG, val);
}

int
EMG_ADS1293::get_R1(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, R1_RATE_REG, 1, &val);
    return ((val >> ch) & 0b1) ? 2 : 4;
}

int
EMG_ADS1293::config_R2(uint8_t R2)
{
    bool R3_val =
        (m_regs[R3_RATE_CH0_REG] == 0b10 || m_regs[R3_RATE_CH0_REG] == 0b1000)
            ? false
            : true;
    if(R2 < 5)
    {
        R2 = 0b0001; //4
        m_fast_adc_max = 0x8000;
        for(int i = 0; i < 3; i++)
            m_precise_adc_max[i] = R3_val ? 0x800000 : 0xF30000;
    }
    else if(R2 < 6)
    {
        R2 = 0b0010; //5
        m_fast_adc_max = 0xC350;
        for(int i = 0; i < 3; i++)
            m_precise_adc_max[i] = R3_val ? 0xC35000 : 0xB964F0;
    }
    else if(R2 < 8)
    {
        R2 = 0b0100; //6
        m_fast_adc_max = 0xF300;
        for(int i = 0; i < 3; i++)
            m_precise_adc_max[i] = R3_val ? 0xF30000 : 0xE6A900;
    }
    else
    {
        R2 = 0b1000; //8
        m_fast_adc_max = 0x8000;
        for(int i = 0; i < 3; i++)
            m_precise_adc_max[i] = R3_val ? 0x800000 : 0xF30000;
    }
    m_regs[R2_RATE_REG] = R2;
    return m_controller->writeReg(this->id, R2_RATE_REG, R2);
}

int
EMG_ADS1293::get_R2(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, R2_RATE_REG, 1, &val);
    switch(val & 0b1111)
    {
    case 0b0001:
        return 4;
    case 0b0010:
        return 5;
    case 0b0100:
        return 6;
    case 0b1000:
        return 8;
    default:
        return 0;
    }
}

int
EMG_ADS1293::config_R3(int ch, uint8_t R3)
{
    if(R3 < 6)
        R3 = 0b00000001; //4
    else if(R3 < 8)
        R3 = 0b00000010; //6
    else if(R3 < 11)
        R3 = 0b00000100; //8
    else if(R3 < 15)
        R3 = 0b00001000; //12
    else if(R3 < 25)
        R3 = 0b00010000; //16
    else if(R3 < 49)
        R3 = 0b00100000; //32
    else if(R3 < 97)
        R3 = 0b01000000; //64
    else
        R3 = 0b10000000; //128

    //change conv max adc
    bool R3_val = (R3 == 0b10 || R3 == 0b1000) ? false : true;
    if(m_regs[R2_RATE_REG] == 0b0010)
        m_precise_adc_max[ch] = R3_val ? 0xC35000 : 0xB964F0;
    else if(m_regs[R2_RATE_REG] == 0b0100)
        m_precise_adc_max[ch] = R3_val ? 0xF30000 : 0xE6A900;
    else
        m_precise_adc_max[ch] = R3_val ? 0x800000 : 0xF30000;

    m_regs[R3_RATE_CH0_REG + ch] = R3;
    return m_controller->writeReg(this->id, R3_RATE_CH0_REG + ch, R3);
}

int
EMG_ADS1293::get_R3(int ch)
{
    uint8_t val = 0;
    m_controller->readReg(this->id, R3_RATE_CH0_REG + ch, 1, &val);
    switch(val)
    {
    case 0b00000001:
        return 4;
    case 0b00000010:
        return 6;
    case 0b00000100:
        return 8;
    case 0b00001000:
        return 12;
    case 0b00010000:
        return 16;
    case 0b00100000:
        return 32;
    case 0b01000000:
        return 64;
    case 0b10000000:
        return 128;
    default:
        return 0;
    }
}

double
EMG_ADS1293::precise_value(int ch)
{
    double val = conv(ch, *m_precise_value[ch]);
    return val;
};

double
EMG_ADS1293::fast_value(int ch)
{
    return conv(m_fast_value[ch]);
};

double
EMG_ADS1293::read_fast_value(int ch)
{
    m_controller->readReg(this->id, DATA_CH0_PACE_REG + 2 * ch, 2,
                      &(m_fast_value[ch]));
    return conv(m_fast_value[ch]);
}

double
EMG_ADS1293::read_precise_value(int ch)
{
    m_controller->readReg(this->id, DATA_CH0_ECG_REG + 3 * ch, 3,
                      m_precise_value[ch]);
    return conv(ch, *m_precise_value[ch]);
}

double
EMG_ADS1293::conv(uint16_t val)
{
    // uint16_t lim = 0x8000;
    // if((uint16_t)val > lim)
    // std::cout << "16conv " << std::hex << __builtin_bswap16(val) << " "
    //           << m_fast_adc_max << std::dec << std::endl;
    // std::cout << "conv " << std::hex << __builtin_bswap16(val) << " "
    //           << std::dec
    //           << (__builtin_bswap16(val) * 1. / m_fast_adc_max - 0.5) *
    //                  4.8 / 3.5 * 1000
    //           << std::endl;
    return (__builtin_bswap16(val) * 1. / m_fast_adc_max - 0.5) * 4.8 / 3.5 *
           1000;
}

double
EMG_ADS1293::conv(int ch, int32_t val)
{
    //m_regs[DATA_CH0_ECG_REG+3] =0x01;
    //std::cout << "32conv " << std::hex << (__builtin_bswap32(val) >> 8) << " "
    //           << m_precise_adc_max[ch] << std::dec << std::endl;
    return ((__builtin_bswap32(val) >> 8) * 1. / m_precise_adc_max[ch] - 0.5) *
           4.8 / 3.5 * 1000;
}

int
EMG_ADS1293::get_error()
{
    return m_controller->readReg(this->id, ERROR_LOD_REG, 7,
                             &m_regs[ERROR_LOD_REG]);
    //return (Error *)&(m_regs[ERROR_LOD_REG]);
}

std::string
EMG_ADS1293::error_range_str()
{
    std::string str;
    for(int j = 0; j < 3; j++)
    {
        uint8_t err_byte = m_regs[ERROR_RANGE1_REG + j];
        for(int i = 0; i < 8; i++)
            if(1 & (err_byte >> i))
                switch(i)
                {
                case 0:
                    str +=
                        "INA(" + std::to_string(j) + ") output out-of-range | ";
                    break;
                case 1:
                    str += "INA(" + std::to_string(j) +
                           ") (+) output near (+) rail | ";
                    break;
                case 2:
                    str += "INA(" + std::to_string(j) +
                           ") (+) output near (-) rail | ";
                    break;
                case 3:
                    str += "INA(" + std::to_string(j) +
                           ") (-) output near (+) rail | ";
                    break;
                case 4:
                    str += "INA(" + std::to_string(j) +
                           ") (-) output near (-) rail | ";
                    break;
                case 6:
                    str += "Sigma-delta modulator over range |";
                    break;
                }
    }
    if(str.size() > 2)
    {
        str.pop_back();
        str.pop_back();
    }
    return str;
}

std::string
EMG_ADS1293::error_status_str()
{
    std::string str;
    uint8_t err_byte = m_regs[ERROR_STATUS_REG];
    for(int i = 0; i < 8; i++)
        if(1 & (err_byte >> i))
            switch(i)
            {
            case 0:
                str += "Common-mode level out-of-range | ";
                break;
            case 1:
                str += "Right leg drive near rail | ";
                break;
            case 2:
                str += "Low battery | ";
                break;
            case 3:
                str += "Lead off detected | ";
                break;
            case 4:
                str += "Channel 1 out-of-range error | ";
                break;
            case 5:
                str += "Channel 2 out-of-range error | ";
                break;
            case 6:
                str += "Channel 3 out-of-range error | ";
                break;
            case 7:
                str += "Digital synchronization error | ";
                break;
            }
    if(str.size() > 2)
    {
        str.pop_back();
        str.pop_back();
    }
    return str;
}

} // namespace ClvHd
