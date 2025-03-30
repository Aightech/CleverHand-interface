#ifndef CLVHDCONTROLLER_H
#define CLVHDCONTROLLER_H

#include <cmath>
#include <cstring>
#include <iostream> // std::cout, std::endl
#include <string>
#include <vector>

#include <stdint.h> // uint8_t, uint16_t, uint32_t, uint64_t
#include <stdio.h>  // Standard input/output definitions

#include "strANSIseq.hpp"
#include "clvHd_module.hpp" // Module class


namespace ClvHd
{

/**
 * @brief The CleverHand Controller board class
 * @author Alexis Devillard
 * @date 2022
 */
class Controller : virtual public ESC::CLI
{
    // using clk = std::chrono::system_clock;
    // using sec = std::chrono::duration<double>;

    public:
    Controller(int verbose = -1)
        : ESC::CLI(verbose, "ClvHd-Controller"){};
    virtual ~Controller(){};

    virtual uint8_t
    setup()=0;

    int
    readCmd(uint8_t id,
            uint8_t n_cmd,
            uint8_t *cmd,
            uint8_t size,
            const void *buff,
            uint64_t *timestamp = nullptr)
    {
        return readCmd_multi(((uint32_t)1) << id, n_cmd, cmd, size, buff,
                             timestamp);
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
                  uint64_t *timestamp = nullptr)=0;

    /**
     * @brief writeReg write size byte to the module with the given id.
     *
     * @param id Id of the module to write to.
     * @param n_cmd Number of bytes of the write command.
     * @param cmd Write command to send to the module.
     * @param size Number of bytes to write.
     * @param data Data to write.
     * @return int Number of bytes written.
     */
    int
    writeCmd(uint8_t id,
             uint8_t n_cmd,
             uint8_t *cmd,
             uint8_t size = 0,
             const void *data = nullptr)
    {
        return writeCmd_multi(((uint32_t)1) << id, n_cmd, cmd, size, data);
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
                   const void *data = nullptr)=0;

    virtual void setRGB(int id_module, RGBColor &color)=0;

    /**
     * @brief Get the version of the controller board.
     * @return The version of the controller board as a std::string. If an empty string is returned, an error occured.
     */
    std::string
    getVersion(uint8_t *major = nullptr, uint8_t *minor = nullptr);

    operator std::string() const { return "Controller board"; };

};
} // namespace ClvHd
#endif
