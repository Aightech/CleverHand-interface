#ifndef __CLVHDCONTROLLER_SERIAL_HPP
#define __CLVHDCONTROLLER_SERIAL_HPP
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

#include "clvHd_module.hpp" // Module class
#include <serial_client.hpp>

#define CLVHD_PACKET_SIZE 6
#define CLVHD_BUFFER_SIZE 1024

namespace ClvHd
{
class Module;

/**
 * @brief The CleverHand Controller board class
 *
 * This class is the main class of the library.
 * It is used to communicate with the controller board and to read/write registers of each modules.
 *
 * @author Alexis Devillard
 * @date 2022
 */
class SerialController : public Controller
{
    // using clk = std::chrono::system_clock;
    // using sec = std::chrono::duration<double>;

    public:
    SerialController(int verbose = -1)
        : ESC::CLI(verbose, "ClvHd-Controller"), m_serial(verbose) {};
    ~SerialController()
    {
        sendCmd('z');
        m_serial.close_connection();
    };

    void
    open(const char *path, int baud = 460800, int flags = O_RDWR | O_NOCTTY)
    {
        m_serial.open_connection(path, baud, flags);
    };

    virtual void
    setRGB(int id_module, RGBColor &color)
    {
        uint8_t msg[6] = {'i',          (uint8_t)id_module, 0,
                          color.red[0], color.green[0],     color.blue[0]};
        sendCmd(msg, 6);
    };

    uint8_t
    setup()
    {
        logln("Setup controller board", true);
        sendCmd('s');
        uint8_t nb = 0;
        int n = readReply(&nb);
        logln("Number of modules found: " + std::to_string(nb), true);
        if(n == 1)
            return nb;
        else
            return -1; // Error
    };

    int
    sendCmd(uint8_t cmd)
    {
        return sendCmd(&cmd, 1);
    };

    virtual int
    sendCmd(uint8_t *data, size_t size)
    {
        if((size_t)m_serial.writeS(data, size) != size)
        {
            logln("Error sending command", true);
            return -1;
        }
        return size;
    };

    /**
     * @brief readReply Read a reply from the controller board. The reply contains a timestamp, a size and the data.
     *
     * @param buff Buffer to store the data.
     * @param timestamp Timestamp of the reply.
     * @return int Number of bytes read.
     */
    virtual int
    readReply(uint8_t *buff, uint64_t *timestamp = nullptr)
    {
        // Read the timestamp and the size of the data (8 bytes + 1 byte)
        // printf("readReply\n");
        int n = m_serial.readS(m_buffer, 9);
        // printf("n: %d \t size: %d\n", n, m_buffer[8]);
        if(n == 9)
        {
            // uint64_t ts = 0;
            // uint8_t sbuff = 0;
            // uint8_t val = 0;
            // ts = *(uint64_t *)m_buffer;
            // sbuff = m_buffer[8];
            // val = m_buffer[9];
            // printf("ts: %lu\n", ts);
            // printf("sbuff: %d\n", sbuff);
            // printf("val: %d\n", val);

            if(timestamp != nullptr)
                *timestamp = *(uint64_t *)m_buffer;
            n = m_serial.readS(buff, m_buffer[8]);
            // printf("n: %d \t size: %d\n", n, m_buffer[8]);
            return n;
        }
        else
            return -1;
    };

    /**
     * @brief readReg_multi read size byte to the modules given by the mask_id.
     *
     * @param mask_id Mask of the modules to read from.
     * @param n_cmd Number of bytes of the read command.
     * @param cmd Read command to send to the module.
     * @param size Number of bytes to read.
     * @param buff Buffer to store the data.
     * @return int Number of bytes read.
     */
    virtual int
    readCmd_multi(uint32_t mask_id,
                  uint8_t n_cmd,
                  uint8_t *cmd,
                  uint8_t size,
                  const void *buff,
                  uint64_t *timestamp = nullptr) override
    {
        uint8_t msg[6];
        *(uint32_t *)msg = mask_id;
        msg[4] = size;
        msg[5] = n_cmd;
        sendCmd('r');
        sendCmd(msg, 6);
        sendCmd(cmd, n_cmd);
        return readReply((uint8_t *)buff, timestamp);
    };

    /**
     * @brief writeReg_multi write size byte to the modules given by the mask_id.
     *
     * @param mask_id Mask of the modules to write to.
     * @param n_cmd Number of bytes of the write command.
     * @param cmd Write command to send to the module.
     * @param size Number of bytes to write.
     * @param data Data to write.
     * @return int Number of bytes written.
     */
    virtual int
    writeCmd_multi(uint32_t mask_id,
                   uint8_t n_cmd,
                   uint8_t *cmd,
                   uint8_t size = 0,
                   const void *data = nullptr) override
    {
        uint8_t msg[6];
        *(uint32_t *)msg = mask_id;
        msg[4] = size;
        msg[5] = n_cmd;
        sendCmd('w');
        sendCmd(msg, 6);
        sendCmd(cmd, n_cmd);
        if(size > 0)
            return sendCmd((uint8_t *)data, size);
        else
            return 0;
    };

    /**
     * @brief Get the number of modules connected to the controller board.
     * @return The number of modules connected to the controller board.
     */
    uint8_t
    getNbModules()
    {
        sendCmd('n'); // Request the number of modules and their types
        uint8_t nb = 0;
        int n = readReply(&nb);
        if(n == 1)
            return nb;
        else
            return -1; // Error
    };

    /**
     * @brief Check if the connection to the controller board is established
     * @return True if the connection is established, false otherwise.
     */
    bool
    test_connection()
    {
        uint8_t arr[3] = {1, 2, 3};
        sendCmd('m');
        sendCmd(arr, 3);
        uint8_t ans[3];
        if(readReply(ans) == 3)
            return ans[0] == arr[0] && ans[1] == arr[1] && ans[2] == arr[2];
        else
            return false;
    };

    /**
     * @brief Get the version of the controller board.
     * @return The version of the controller board as a std::string. If an empty string is returned, an error occured.
     */
    std::string
    getVersion(uint8_t *major = nullptr, uint8_t *minor = nullptr)
    {
        sendCmd('v');
        uint8_t ans[2];
        if(readReply(ans) == 2)
        {
            if(major != nullptr)
                *major = ans[0];
            if(minor != nullptr)
                *minor = ans[1];
            return std::to_string((int)ans[0]) + "." +
                   std::to_string((int)ans[1]);
        }
        else
            return "";
    };

    operator std::string() const { return "Controller board"; };

    private:
    uint8_t m_buffer[CLVHD_BUFFER_SIZE];
    Communication::Serial m_serial;
};
} // namespace ClvHd
#endif // __CLVHDCONTROLLER_SERIAL_HPP
