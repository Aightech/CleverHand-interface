#ifndef __CLV_HD_MSG_HPP__
#define __CLV_HD_MSG_HPP__

#include <vector>

namespace ClvHd
{
    struct Value
    {
        enum Type
        {
            Sensor,
            Actuator,
        };

        Type type;
        uint64_t time_s;
        uint64_t time_ns;
        std::vector<double> data;
    };

    struct RGBColor
    {
        std::vector<uint8_t> red;
        std::vector<uint8_t> green;
        std::vector<uint8_t> blue;
    };

} // namespace ClvHd


#endif // __CLV_HD_MSG_HPP__