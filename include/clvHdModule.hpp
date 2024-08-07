#ifndef __CLV_HD_MODULE_HPP__
#define __CLV_HD_MODULE_HPP__

#define CLVHD_REGISTER_SIZE 256
#include <string>

namespace ClvHd
{
class Controller;
class Module
{
    public:
    Module(){};
    ~Module(){};

    std::string 
    get_type()
    {
        return m_type;
    };

    int8_t id=-1; // Id of the module [0-31], -1 if not set
    protected:
    Controller *m_controller;
    std::string m_type;
};
} // namespace ClvHd

#endif // __CLV_HD_MODULE_HPP__