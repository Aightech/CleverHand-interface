#ifndef __CLV_HD_UCTRL_HPP__
#define __CLV_HD_UCTRL_HPP__

#include "com_client.hpp" // COM class

namespace ClvHd
{

  
class uCtrl : public Communication::Client
{
    public:
    uCtrl(int verbose = -1) : ESC::CLI(verbose, "ClvHd-uCtrl"){};

    /**
     * @brief readReg read size byte starting from reg address from the module with the given id.
     *
     * @param id Id of the module to read from.
     * @param reg Start address of the data to read.
     * @param size Number of bytes to read.
     * @param buff Buffer to store the data.
     * @return int Number of bytes read.
     */
    int
    readReg(uint8_t id, uint8_t reg, size_t size, const void *buff);

    int8_t
    readReg8(uint8_t id, uint8_t reg);
    int16_t
    readReg16(uint8_t id, uint8_t reg);
    int32_t
    readReg24(uint8_t id, uint8_t reg);
    int32_t
    readReg32(uint8_t id, uint8_t reg);
    int64_t
    readReg64(uint8_t id, uint8_t reg);
    float
    readRegFloat(uint8_t id, uint8_t reg);
    double
    readRegDouble(uint8_t id, uint8_t reg);

    /**
     * @brief writeReg write one byte to reg address to the module with the given id.
     *
     * @param id Id of the module to write to.
     * @param reg Start address of the data to write.
     * @param val address of the value to write.
     * @param size Number of bytes to write.
     * @return int Number of bytes written.
     */
    int
    writeReg(uint8_t id, uint8_t reg, char *val, size_t size = 1);

    /**
     * @brief Get the number of modules connected to the master board.
     * @return The number of modules connected to the master board.
     */
    int
    getNbModules()
    {
        char msg[6] = {'n', 0, 0, 0};
        writeS(msg, 4, true);
        int n = readS((uint8_t *)msg, 4, true);
        if(n == 4)
            return msg[0];
        else
            return -1;
    };

    /**
     * @brief Check if the connection to the master board is established
     * @return True if the connection is established, false otherwise.
     */
    bool
    test_connection()
    {
        char msg[6] = {'m', 1, 2, 3};
        char ans[6];
        writeS(msg, 4, true);
        int n = readS((uint8_t *)ans, 6, true);
        if(n != 6)
            return false;
        else
            for(int i = 0; i < 4; i++)
                if(ans[i] != msg[i])
                    return false;
        return true;
    };

    /**
     * @brief Trigger a blinking effect on the module with the given id.
     * @param id Id of the module to blink.
     * @param dt_cs Duration of the blinking in centiseconds
     * @param nb_blink Number of blinking.
     */
    void
    blink(uint8_t id, uint8_t dt_cs, uint8_t nb_blink)
    {
        uint8_t msg[6] = {'b', id, dt_cs, nb_blink};
        writeS(msg, 4, true);
    }

    /**
     * @brief Get the version of the master board.
     * @param i Index of the version value. For i = 1, the major version is returned. For i = 2, the minor version is returned. For i = 0, a short composed of the major and minor version is returned.
     * @return The version of the master board. If 0 is returned, an error occured.
     */
    uint16_t
    getVersion(int i)
    {
        char msg[6] = {'v', 0, 0, 0};
        writeS(msg, 4, true);
        if(readS((uint8_t *)msg, 4, true) == 4)
        {
            if(i == 0)
                return *(uint16_t *)(msg);
            if(i == 1)
                return msg[0];
            if(i == 2)
                return msg[1];
        }
        return 0;
    };

    /**
     * @brief Get the version of the master board.
     * @return The version of the master board as a std::string. If an empty string is returned, an error occured.
     */
    std::string
    getVersion()
    {
        char msg[6] = {'v', 0, 0, 0};
        writeS(msg, 4, true);
        if(readS((uint8_t *)msg, 4, true) == 4)
            return std::to_string((int)msg[0]) + "." +
                   std::to_string((int)msg[1]);
        else
            return "";
    }

    private:
    bool m_streaming = false;
    ADS1293_Reg m_streaming_reg;
    size_t m_streaming_size;
    std::vector<std::pair<int, uint8_t>> m_streaming_channels;
    uint8_t m_buffer[512];

    Connection m_connection;
};

  
}; // namespace ClvHd

#endif // __CLV_HD_UCTRL_HPP__
