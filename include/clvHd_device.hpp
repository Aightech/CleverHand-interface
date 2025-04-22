#ifndef __CLV_HD_DEVICE_HPP__
#define __CLV_HD_DEVICE_HPP__

#include "clvHd_controller.hpp"
#include "clvHd_module.hpp"

#include "clvHd_controller_serial.hpp"
#include "clvHd_controller_wifi.hpp"

namespace ClvHd
{
class Device : virtual public ESC::CLI
{

    public:
    Device(int verbose = -1) : ESC::CLI(verbose, "ClvHd-Device") {};
    ~Device() {};

    uint8_t
    initSerial(const char *path,
               int baud = 460800,
               int flags = O_RDWR | O_NOCTTY)
    {
        if(controller != nullptr)
            delete controller;
        SerialController *c = new SerialController(m_verbose);
        c->open(path, baud, flags);
        controller = c;
        return this->setup();
    };

    uint8_t
    initWifi()
    {
        if(controller != nullptr)
            delete controller;
        controller = new WifiController(m_verbose);
        //cast to WifiController
        WifiController *clvhd = static_cast<WifiController *>(controller);
        clvhd->start();
        while(clvhd->nbClients() < 1)
        {
            std::cout << "Waiting for a client to connect..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        return this->setup();
    };

    uint8_t
    setup()
    {
        int nb_modules = controller->setup();
        logln("Number of modules found: " + std::to_string(nb_modules), true);
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
        logln("Adding module " + std::to_string(module->id), true);
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