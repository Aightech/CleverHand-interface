#include <clvHd_module_ADS1293EMG.hpp>

namespace ClvHd
{
using namespace ESC;

uint32_t EMG_ADS1293::modules_mask = 0;
uint8_t EMG_ADS1293::nb_modules = 0;
ESC::CLI EMG_ADS1293::s_cli = ESC::CLI(-1, "EMG_ADS1293");

EMG_ADS1293::EMG_ADS1293(Controller *controller, int id, int verbose)
    : ESC::CLI(verbose, "EMG_" + std::to_string(id)), Module(controller, id, verbose)
{
    this->id = id;
    this->m_controller = controller;
    this->m_type = "EMG_ADS1293";
    this->typed = true;
    // logln("Initialised", true);
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

    sensorValue.data.resize(3);
    sensorValue.time_s = 0;
    sensorValue.time_ns = 0;
};

EMG_ADS1293::~EMG_ADS1293() { this->writeReg(CONFIG_REG, 0x02); }

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
    int n = this->set_mode(POWER_DOWN);
    n += this->config_clock(false, clk_src, false);
    n += this->route_channel(0, route_table[0][0], route_table[0][1]);
    n += this->route_channel(1, route_table[1][0], route_table[1][1]);
    n += this->route_channel(2, route_table[2][0], route_table[2][1]);

    // this->route_channel_test(0, false, true);
    // this->route_vbat(true, false, false);

    n += this->enable_ADC(chx_enable[0], chx_enable[1], chx_enable[2]);
    n += this->config_resolution(chx_high_res[0], chx_high_res[1],
                                 chx_high_res[2]);
    n += this->config_frequence(chx_high_freq[0], chx_high_freq[1],
                                chx_high_freq[2]);
    this->set_filters(R1, R2, R3);

    // logln("Seting n=" + std::to_string(n), true);
    this->config_clock(true, clk_src, false);

    bool ret = is_clock_ext();
    std::string clk_src_s = std::string(ret ? "extern" : "intern");

    this->update_adc_max();

    this->get_filters(R1, &R2, R3);

    // int range[3] = {0, 1, 2};

    logln("Seting up ADS1293 : " + ((n == 8) ? fstr(" OK", {BOLD, FG_GREEN})
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
        log("(-)" + std::to_string(get_route_neg(i)) + " (+)" +
            std::to_string(get_route_pos(i)) + "|");
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
        // R1[i] = get_R1(i);
        log("   " + std::string(3 - std::to_string(R1[i]).size(), ' ') +
            std::to_string(R1[i]) + "   |");
    }
    logln("");

    // R2 = get_R2();
    logln("   R2      |             " +
          std::string(3 - std::to_string(R2).size(), ' ') + std::to_string(R2) +
          "             |");

    log("   R3      |", true);
    for(int i : {0, 1, 2})
    {
        // R3[i] = get_R3(i);
        log("   " + std::string(3 - std::to_string(R3[i]).size(), ' ') +
            std::to_string(R3[i]) + "   |");
    }
    logln("");
    return n;
}

int
EMG_ADS1293::route_channel(uint8_t channel, uint8_t pos_in, uint8_t neg_in)
{
    pos_in = (pos_in > 6) ? 6 : pos_in;
    neg_in = (neg_in > 6) ? 6 : neg_in;

    channel = (channel > 2) ? 2 : channel;
    uint8_t val = pos_in | (neg_in << 3);
    if(pos_in == neg_in)
        val |= 0xc0;
    m_regs[FLEX_CH0_CN_REG + channel] = val;
    return this->writeReg(FLEX_CH0_CN_REG + channel, val);
}

int
EMG_ADS1293::route_channel_test(uint8_t channel, bool pos_test, bool neg_test)
{
    channel = (channel > 2) ? 2 : channel;
    uint8_t val = (pos_test ? 0x80 : 0) | (neg_test ? 0x40 : 0);
    m_regs[FLEX_CH0_CN_REG + channel] = val;
    return this->writeReg(FLEX_CH0_CN_REG + channel, val);
}

int
EMG_ADS1293::route_vbat(bool ch1, bool ch2, bool ch3)
{
    uint8_t val = (ch1 ? 0x1 : 0x0) | (ch2 ? 0x2 : 0x0) | (ch3 ? 0x4 : 0x0);
    m_regs[FLEX_VBAT_CN_REG] = val;
    return this->writeReg(FLEX_VBAT_CN_REG, val);
}

int
EMG_ADS1293::get_route_neg(int ch)
{
    uint8_t val = 0;
    this->readReg(FLEX_CH0_CN_REG + ch, 1, &val);
    // printf("get_route_neg: 0x%02X\n", val);
    return ((val & 0b111000) >> 3);
}

int
EMG_ADS1293::get_route_pos(int ch)
{
    uint8_t val = 0;
    this->readReg(FLEX_CH0_CN_REG + ch, 1, &val);
    return (val & 0b111);
}

int
EMG_ADS1293::set_mode(Mode mode)
{
    m_mode = mode;
    m_regs[CONFIG_REG] = mode;
    return this->writeReg(CONFIG_REG, mode);
}

EMG_ADS1293::Mode
EMG_ADS1293::get_mode()
{
    uint8_t val = 0;
    this->readReg(CONFIG_REG, 1, &val);
    return (Mode)(val);
}

int
EMG_ADS1293::config_clock(bool start, CLK_SRC src, bool en_output)
{
    uint8_t val = (start ? 0x4 : 0x0) | (src << 1) | (en_output ? 0x1 : 0x0);
    m_regs[OSC_CN_REG] = val;
    return this->writeReg(OSC_CN_REG, val);
}

bool
EMG_ADS1293::is_clock_started()
{
    uint8_t val = 0;
    this->readReg(OSC_CN_REG, 1, &val);
    // printf("is_clock_started: 0x%02X\n", val);
    return (val >> 2) & 0b1;
}

bool
EMG_ADS1293::is_clock_ext()
{
    uint8_t val = 0;
    this->readReg(OSC_CN_REG, 1, &val);
    bool ret = (val >> 1) & 0b1;
    // printf("is_clock_ext: 0x%02X\n", val);
    //logln("is_clock_ext: " + std::to_string(ret));
    return ret;
}

bool
EMG_ADS1293::is_clock_output_enabled()
{
    uint8_t val = 0;
    this->readReg(OSC_CN_REG, 1, &val);
    return (val & 0b1);
}

int
EMG_ADS1293::enable_ADC(bool ch0, bool ch1, bool ch2)
{
    m_adc_enabled[0] = ch0;
    m_adc_enabled[1] = ch1;
    m_adc_enabled[2] = ch2;
    uint8_t val =
        (ch0 ? 0 : 0b001001) | (ch1 ? 0 : 0b010010) | (ch2 ? 0 : 0b100100);
    m_regs[AFE_SHDN_CN_REG] = val;
    return this->writeReg(AFE_SHDN_CN_REG, val);
}

bool
EMG_ADS1293::is_ADC_enabled(int ch)
{
    uint8_t val = 0;
    this->readReg(AFE_SHDN_CN_REG, 1, &val);
    uint8_t mask = 0b001001;
    return !(val & (mask << ch));
}

int
EMG_ADS1293::enable_SDM(bool ch0, bool ch1, bool ch2)
{
    uint8_t val = (m_regs[AFE_SHDN_CN_REG] & 0b111) | (ch0 ? 0 : 0b1000) |
                  (ch1 ? 0 : 0b10000) | (ch2 ? 0 : 0b100000);
    m_regs[AFE_SHDN_CN_REG] = val;
    return this->writeReg(AFE_SHDN_CN_REG, val);
}

bool
EMG_ADS1293::is_SDM_enabled(int ch)
{
    uint8_t val = 0;
    this->readReg(AFE_SHDN_CN_REG, 1, &val);
    return !((val >> (3 + ch)) & 0b1);
}

int
EMG_ADS1293::enable_INA(bool ch0, bool ch1, bool ch2)
{
    uint8_t val = (m_regs[AFE_SHDN_CN_REG] & 0b111000) | (ch0 ? 0 : 0b1) |
                  (ch1 ? 0 : 0b10) | (ch2 ? 0 : 0b100);
    m_regs[AFE_SHDN_CN_REG] = val;
    return this->writeReg(AFE_SHDN_CN_REG, val);
}

bool
EMG_ADS1293::is_INA_enabled(int ch)
{
    uint8_t val = 0;
    this->readReg(AFE_SHDN_CN_REG, 1, &val);
    return !((val >> (ch)) & 0b1);
}

int
EMG_ADS1293::config_resolution(bool ch0_high_res,
                               bool ch1_high_res,
                               bool ch2_high_res)
{
    uint8_t val = (m_regs[AFE_RES_REG] & 0b00111000) |
                  (ch0_high_res ? 0b1 : 0) | (ch1_high_res ? 0b010 : 0) |
                  (ch2_high_res ? 0b100 : 0);
    m_regs[AFE_RES_REG] = val;
    return this->writeReg(AFE_RES_REG, val);
}

bool
EMG_ADS1293::is_high_res_enabled(int ch)
{
    uint8_t val = 0;
    this->readReg(AFE_RES_REG, 1, &val);
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
    return this->writeReg(AFE_RES_REG, val);
}

bool
EMG_ADS1293::is_high_freq_enabled(int ch)
{
    uint8_t val = 0;
    this->readReg(AFE_RES_REG, 1, &val);
    return ((val >> (ch + 3)) & 0b1);
}

void
EMG_ADS1293::get_filters(int R1[3], int *R2, int R3[3])
{
    this->readReg(R1_RATE_REG, 1, &m_regs[R1_RATE_REG]);
    for(int i = 0; i < 3; i++)
    {
        R1[i] = ((m_regs[R1_RATE_REG] >> i) & 0b1) ? 2 : 4;
        this->readReg(R3_RATE_CH0_REG + i, 1, &m_regs[R3_RATE_CH0_REG + i]);
        switch(m_regs[R3_RATE_CH0_REG + i])
        {
        case 0b00000001:
            R3[i] = 4;
            break;
        case 0b00000010:
            R3[i] = 6;
            break;
        case 0b00000100:
            R3[i] = 8;
            break;
        case 0b00001000:
            R3[i] = 12;
            break;
        case 0b00010000:
            R3[i] = 16;
            break;
        case 0b00100000:
            R3[i] = 32;
            break;
        case 0b01000000:
            R3[i] = 64;
            break;
        case 0b10000000:
            R3[i] = 128;
            break;
        default:
            R3[i] = 0;
            break;
        }
    }
    this->readReg(R2_RATE_REG, 1, &m_regs[R2_RATE_REG]);
    switch(m_regs[R2_RATE_REG] & 0b1111)
    {
    case 0b0001:
        *R2 = 4;
        break;
    case 0b0010:
        *R2 = 5;
        break;
    case 0b0100:
        *R2 = 6;
        break;
    case 0b1000:
        *R2 = 8;
        break;
    default:
        *R2 = 0;
        break;
    }
}

void
EMG_ADS1293::update_adc_max()
{
    int R1[3], R2, R3[3];
    get_filters(R1, &R2, R3);

    for(int i = 0; i < 3; i++)
    {
        switch(R2)
        {
        case 4:
            m_fast_adc_max = 0x8000;
            m_precise_adc_max[i] = 0x800000;
            if(R3[i] == 6 || R3[i] == 12)
                m_precise_adc_max[i] = 0xF30000;
            break;
        case 5:
            m_fast_adc_max = 0xC350;
            m_precise_adc_max[i] = 0xC35000;
            if(R3[i] == 8 || R3[i] == 16)
                m_precise_adc_max[i] = 0xB964F0;
            break;
        case 6:
            m_fast_adc_max = 0xF300;
            m_precise_adc_max[i] = 0xF30000;
            if(R3[i] == 8 || R3[i] == 16)
                m_precise_adc_max[i] = 0xE6A900;
            break;
        case 8:
            m_fast_adc_max = 0x8000;
            m_precise_adc_max[i] = 0x800000;
            if(R3[i] == 6 || R3[i] == 12)
                m_precise_adc_max[i] = 0xF30000;
            break;
        }
    }
}

void
EMG_ADS1293::set_filters(int R1[3], int R2, int R3[3])
{
    uint8_t val = 0;
    for(int i = 0; i < 3; i++) { val |= ((R1[i] == 2) ? 0b1 : 0) << i; }
    m_regs[R1_RATE_REG] = val;
    this->writeReg(R1_RATE_REG, val);

    val = 0;
    switch(R2)
    {
    case 4:
        val = 0b0001;
        break;
    case 5:
        val = 0b0010;
        break;
    case 6:
        val = 0b0100;
        break;
    case 8:
        val = 0b1000;
        break;
    default:
        val = 0;
        break;
    }
    m_regs[R2_RATE_REG] = val;
    this->writeReg(R2_RATE_REG, val);

    for(int i = 0; i < 3; i++)
    {
        val = 0;
        switch(R3[i])
        {
        case 4:
            val = 0b00000001;
            break;
        case 6:
            val = 0b00000010;
            break;
        case 8:
            val = 0b00000100;
            break;
        case 12:
            val = 0b00001000;
            break;
        case 16:
            val = 0b00010000;
            break;
        case 32:
            val = 0b00100000;
            break;
        case 64:
            val = 0b01000000;
            break;
        case 128:
            val = 0b10000000;
            break;
        default:
            val = 0;
            break;
        }
        m_regs[R3_RATE_CH0_REG + i] = val;
        this->writeReg(R3_RATE_CH0_REG + i, val);
    }
    update_adc_max();
}

double
EMG_ADS1293::precise_value(int ch, bool converted)
{
    if(!m_adc_enabled[ch])
        return 0;
    if(converted)
        return conv(ch, *m_precise_value[ch]);
    else
        return *m_precise_value[ch];
};

double
EMG_ADS1293::fast_value(int ch, bool converted)
{
    if(!m_adc_enabled[ch])
        return 0;
    if(converted)
        return conv(m_fast_value[ch]);
    else
        return m_fast_value[ch];
};

double
EMG_ADS1293::read_fast_value(int ch, bool converted)
{
    if(!m_adc_enabled[ch])
        return 0;
    this->readReg(DATA_CH0_PACE_REG + 2 * ch, 2,
                  (uint8_t *)&(m_fast_value[ch]));
    if(converted)
        return conv(m_fast_value[ch]);
    else
        return m_fast_value[ch];
}

double
EMG_ADS1293::read_precise_value(int ch, bool converted)
{
    if(!m_adc_enabled[ch])
        return 0;
    this->readReg(DATA_CH0_ECG_REG + 3 * ch, 3, (uint8_t *)m_precise_value[ch]);
    //logln("read_precise_value, n=" + std::to_string(n) + " reg=" + std::to_string(m_regs[DATA_CH0_ECG_REG + 3 * ch]) + " " + std::to_string(m_regs[DATA_CH0_ECG_REG + 3 * ch + 1]) + " " + std::to_string(m_regs[DATA_CH0_ECG_REG + 3 * ch + 2]), true);
    if(converted)
        return conv(ch, *m_precise_value[ch]);
    else
        return *m_precise_value[ch];
}

// std::string EMG_ADS1293::dump_regs(bool pull)
// {
//     //read all the registers of the module
//     std::string str;
//     for(int i = 0; i < 0x50; i++)
//     {
//         if(pull)
//             m_master->readReg(m_module_id, i, 1, &m_regs[i]);
//         char buf[10];
//         sprintf(buf, "%02x", i);
//         str += "0x" + std::string(buf) ;
//         sprintf(buf, "%02x", m_regs[i]);
//         str += " : 0x" + std::string(buf);
//         sprintf(buf, "%03d", m_regs[i]);
//         str += " : " + std::string(buf) + " | 0b" + byte2bits(m_regs[i]) + "\n";
//     }
//     return str;
// }

double
EMG_ADS1293::conv(uint16_t val)
{
    return (__builtin_bswap16(val) * 1. / m_fast_adc_max - 0.5) * 4.8 / 3.5;
    // return ((__builtin_bswap16(val) * 1. / m_fast_adc_max - 0.5) * 3.246+1)*2.4;
}

double
EMG_ADS1293::conv(int ch, int32_t val)
{
    //m_regs[DATA_CH0_ECG_REG+3] =0x01;
    //std::cout << "32conv " << std::hex << (__builtin_bswap32(val) >> 8) << " "
    //           << m_precise_adc_max[ch] << std::dec << std::endl;
    return ((__builtin_bswap32(val) >> 8) * 1. / m_precise_adc_max[ch] - 0.5) *
           4.8 / 3.5;
    // return (((__builtin_bswap32(val) >> 8) * 1. / m_precise_adc_max[ch] - 0.5) * 3.246+1)*2.4;
}

int
EMG_ADS1293::get_error()
{
    return this->readReg(ERROR_LOD_REG, 7, &m_regs[ERROR_LOD_REG]);
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
