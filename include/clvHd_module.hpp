#ifndef __CLV_HD_MODULE_HPP__
#define __CLV_HD_MODULE_HPP__

#include "clvHd_msg.hpp"
#include "strANSIseq.hpp"
#include <string>

namespace ClvHd
{
class Controller;
class Device;
class Module : public virtual ESC::CLI
{
    public:
    Module(Controller *controller, int id, int verbose = -1)
        : ESC::CLI(verbose, "Module"), id(id), m_controller(controller) {};
    virtual ~Module() {};

    virtual Value &
    readSensor()
    {
        return sensorValue;
    };
    virtual void
    writeActuator(Value &value)
    {
        (void)value;
    };

    void
    sendData(uint8_t *cmdData, size_t size)
    {
        (void)cmdData;
        (void)size;
    };
    void
    readData(uint8_t *cmdData, size_t size, uint8_t *data, size_t dataSize)
    {
        (void)cmdData;
        (void)size;
        (void)data;
        (void)dataSize;
    };

    void
    setRGB(RGBColor &color);

    int
    nbSensors()
    {
        return sensorValue.data.size();
    };
    int
    nbActuators()
    {
        return actuatorValue.data.size();
    };

    virtual bool
    testType()
    {
        return false;
    };

    std::string
    get_type()
    {
        return m_type;
    };

    //<< Operator overload
    friend std::ostream &
    operator<<(std::ostream &os, const Module &m)
    {
        os << "Module type: " << m.m_type << std::endl;
        os << "Sensor value: ";
        for(auto &d : m.sensorValue.data) os << d << " ";
        os << std::endl;
        os << "Actuator value: ";
        for(auto &d : m.actuatorValue.data) os << d << " ";
        os << std::endl;
        return os;
    };
    Controller *
    getController()
    {
        return m_controller;
    };

    int8_t id = -1; // Id of the module [0-31], -1 if not set
    Value sensorValue;
    Value actuatorValue;

    bool typed = false;

    protected:
    Controller *m_controller;
    std::string m_type;
};

class ModulePack : virtual public ESC::CLI
{
    public:
    ModulePack(Device *device, int verbose = -1)
        : ESC::CLI(verbose, "ModulePack"), m_device(device) {};
    ~ModulePack() {};

    virtual void
    setup() {};

    virtual std::vector<Value *> &
    read()
    {
        for(auto &m : modules) m->readSensor();
        return sensorValues;
    };

    virtual void
    write(std::vector<Value *> &values)
    {
        for(size_t i = 0; i < values.size(); i++)
            modules[i]->writeActuator(*values[i]);
    };

    void
    addModule(Module *module)
    {
        modules.push_back(module);
        m_mask |= ((uint32_t)1) << module->id;
        sensorValues.push_back(&module->sensorValue);
        actuatorValues.push_back(&module->actuatorValue);
    };

    std::vector<Module *> modules;
    std::vector<Value *> sensorValues;
    std::vector<Value *> actuatorValues;

    protected:
    uint32_t m_mask = 0;
    Device *m_device;
};

} // namespace ClvHd

#endif // __CLV_HD_MODULE_HPP__