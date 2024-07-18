#ifndef __CLV_HD_DEVICE_HPP__
#define __CLV_HD_DEVICE_HPP__

#include "clvHdController.hpp"
#include "clvHdModule.hpp"

#define CLVHD_NUM_MODULES 32

namespace ClvHd
{
class Device : virtual public ESC::CLI
{
    public:
    Device(int verbose = -1)
        : ESC::CLI(verbose, "ClvHd-Device"), controller(verbose - 1)
    {
        for(int i = 0; i < CLVHD_NUM_MODULES; i++) modules[i] = nullptr;
    };

    ~Device(){};

    void
    setup()
    {
        logln("setup");
        nb_modules = controller.setup();
    };

    Module *modules[CLVHD_NUM_MODULES];
    uint8_t module_types[CLVHD_NUM_MODULES];
    uint8_t nb_modules = 0;
    Controller controller;

    protected:
};

} // namespace ClvHd

#endif // __CLV_HD_DEVICE_HPP__