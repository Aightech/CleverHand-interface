#ifndef __CLV_HD_DEVICE_HPP__
#define __CLV_HD_DEVICE_HPP__

#include "clvHdController.hpp"
#include "clvHdModule.hpp"

#define CLVHD_NUM_MODULES 32

namespace ClvHd
{
class Device: virtual public ESC::CLI
{
    public:
    Device(int verbose = -1) : ESC::CLI(verbose, "ClvHd-Device"){};
    ~Device(){};

    void setup()
    {
        controller.setup();
        nb_modules = controller.getNbModules();
    };

    Module *modules[CLVHD_NUM_MODULES];
    uint8_t module_types[CLVHD_NUM_MODULES];
    uint8_t nb_modules = 0;
    Controller controller;

    protected:

};

} // namespace ClvHd

#endif // __CLV_HD_DEVICE_HPP__