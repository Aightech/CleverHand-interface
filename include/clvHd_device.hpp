#ifndef __CLV_HD_DEVICE_HPP__
#define __CLV_HD_DEVICE_HPP__

#include "clvHd_controller.hpp"
#include "clvHd_module.hpp"

#include "clvHd_controller_mono.hpp"
#include "clvHd_controller_serial.hpp"

namespace ClvHd
{
class Device : virtual public ESC::CLI
{

    public:
    Device(int verbose = -1) : ESC::CLI(verbose, "ClvHd-Device") {};
    ~Device() {};

    uint8_t
    initSerial(const char *path, int baud = 460800, int flags = O_RDWR | O_NOCTTY)
    {
        if(controller != nullptr)
            delete controller;
        SerialController *c = new SerialController(m_verbose);
        c->open(path, baud, flags);
        controller = c;
        return this->setup();
    };

    uint8_t
    initMono()
    {
        if(controller != nullptr)
            delete controller;
        controller = new MonoController(m_verbose);
        return this->setup();
    };

    uint8_t
    setup()
    {
        int nb_modules = controller->setup();
        for(int i = 0; i < nb_modules; i++)
            addModule(new Module(controller, i, m_verbose));
        return nb_modules;
    }

    std::vector<Value *> &
    read()
    {
        for(auto &m : modules) m->readSensor();
        return sensorValues;
    };

    void
    write(std::vector<Value *> &values)
    {
        for(size_t i = 0; i < values.size(); i++)
            modules[i]->writeActuator(*values[i]);
    };

    void
    addModule(Module *module)
    {
        modules.push_back(module);
        sensorValues.push_back(&module->sensorValue);
        actuatorValues.push_back(&module->actuatorValue);
    };

    std::vector<Value *> sensorValues;
    std::vector<Value *> actuatorValues;

    std::vector<Module *> modules;
    Controller *controller = nullptr;
};

} // namespace ClvHd

#endif // __CLV_HD_DEVICE_HPP__