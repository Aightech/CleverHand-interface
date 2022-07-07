#include "clvHdMaster.hpp"
namespace ClvHd
{

Master::~Master()
{
    uint8_t b[6] = {'z', 0, 0, 0};
    this->writeS(b, 4, true);
    this->close_connection();
};

void
Master::printBit(int8_t val)
{
    std::cout << " " << std::flush;
    for(int i = 0; i < 8; i++)
        if(1 & (val >> i))
            std::cout << "[" + std::to_string(i) + "] " << std::flush;
};

int
Master::readReg(uint8_t id, uint8_t reg, size_t size, const void *buff)
{
    char msg[4] = {'r', (char)(0x0f - id), (char)reg, (char)size};
    writeS(msg, 4, true);
    return readS((uint8_t *)buff, size + 2, true); //+2 for CRC
};

int
Master::writeReg(uint8_t id, uint8_t reg, char val)
{
    char msg[4] = {'w', (char)(0x0f - id), (char)reg, (char)val};
    return writeS(msg, 4, true);
};

void
Master::setupEMG(int n_board,
                 int route_table[3][2],
                 bool chx_enable[3],
                 bool chx_high_res[3],
                 bool chx_high_freq[3],
                 int R1[3],
                 int R2,
                 int R3[3])
{
    m_EMG[n_board]->setup(route_table, chx_enable, chx_high_res, chx_high_freq,
                          R1, R2, R3);
}

int
Master::setup()
{
    // //Communication::Client::clean_start();
    std::cout << "> Testing connection: "
              << ESC::fstr(((test_connection()) ? "Established" : "Error"),
                           {ESC::FORMAT::FG_GREEN})
              << std::endl;

    //blink(15, 10, 3);
    int8_t nb_emg = getNbModules();
    std::cout << "> Number of modules: " << (int)nb_emg << std::endl;
    for(int i = 0; i < nb_emg; i++) m_EMG.push_back(new EMG(this, i));
    return nb_emg;
}

bool
Master::data_ready(int id, int channel, bool precise)
{
    uint8_t mask = 1 << (2 + precise * 3 + channel);
    return *(m_EMG[id]->get_regs() + DATA_STATUS_REG) & mask;
}

double
Master::read_precise_EMG(int id, int channel)
{
    return m_EMG[id]->read_precise_value(channel);
}

double
Master::read_fast_EMG(int id, int channel)
{
    return m_EMG[id]->read_fast_value(channel);
}

double
Master::precise_EMG(int id, int channel)
{
    return m_EMG[id]->precise_value(channel);
}

double
Master::fast_EMG(int id, int channel)
{
    return m_EMG[id]->fast_value(channel);
}

void
Master::start_acquisition()
{
    for(int i = 0; i < m_EMG.size(); i++) m_EMG[i]->set_mode(EMG::START_CONV);
}

void
Master::stop_acquisition()
{
    for(int i = 0; i < m_EMG.size(); i++) m_EMG[i]->set_mode(EMG::STANDBY);
}

int
Master::read_all_signal()
{
    int v = 1;
    for(int i = 0; i < m_EMG.size(); i++)
        v *= this->readReg(i, DATA_STATUS_REG, 16,
                           m_EMG[i]->get_regs() + DATA_STATUS_REG);
    return v;
}

std::string
Master::get_error(int id, bool verbose)
{
    std::string str;
    m_EMG[id]->get_error();
    str += m_EMG[id]->error_status_str();
    if(verbose)
        str += m_EMG[id]->error_range_str();

    return str;
}

} // namespace ClvHd
