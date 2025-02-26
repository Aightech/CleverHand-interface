#ifndef __PY_CLV_HD_HPP__
#define __PY_CLV_HD_HPP__

#include "clvHdADS1293EMG.hpp"
#include "clvHdDevice.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <limits>  // For std::numeric_limits

namespace py = pybind11;

namespace ClvHd
{
class pyDevice : public ClvHd::Device
{
    public:
    pyDevice(int verbose = -1)
        : ClvHd::Device(verbose), ESC::CLI(verbose, "pyClvHd-Device") {};

    void
    setupADS1293(py::list _route_table,
                py::list _chx_enable,
                py::list _chx_high_res,
                py::list _chx_high_freq,
                py::list _R1,
                int R2,
                py::list _R3)
    {
        int route_table[3][2];
        for (size_t i = 0; i < 3; ++i) {
            py::list inner_list = _route_table[i].cast<py::list>();  
            for (size_t j = 0; j < 2; ++j) {
                route_table[i][j] = inner_list[j].cast<int>();
            }
        }

        bool chx_enable[3];
        bool chx_high_res[3];
        bool chx_high_freq[3];
        for (size_t i = 0; i < 3; ++i) {
            chx_enable[i] = _chx_enable[i].cast<bool>();
            chx_high_res[i] = _chx_high_res[i].cast<bool>();
            chx_high_freq[i] = _chx_high_freq[i].cast<bool>();
        }

        int R1[3];
        int R3[3];
        for (size_t i = 0; i < 3; ++i) {
            R1[i] = _R1[i].cast<int>();
            R3[i] = _R3[i].cast<int>();
        }

        ClvHd::EMG_ADS1293::setup(*this, chx_enable, route_table, chx_high_res, chx_high_freq, R1, R2, R3);
        
    }


    void
    start_acquisition()
    {
        ClvHd::EMG_ADS1293::start_acquisition(*this);
    }

    py::tuple
    read_all()
    {
        int nb_modules = ClvHd::EMG_ADS1293::nb_modules;
        printf("nb_modules: %d\n", nb_modules);
        double fast[ClvHd::EMG_ADS1293::nb_modules * 3];
        double precise[ClvHd::EMG_ADS1293::nb_modules * 3];
        uint8_t flags[ClvHd::EMG_ADS1293::nb_modules];
        uint64_t timestamp =
            ClvHd::EMG_ADS1293::read_all(*this, fast, precise, flags);
        py::list fast_list;
        py::list precise_list;
        for(int i = 0; i < ClvHd::EMG_ADS1293::nb_modules; i++)
        {
            py::list fast_ch;
            py::list precise_ch;
            for(int j = 0; j < 3; j++)
            {
                //check if the value is available
                if((flags[i]>>(2+j))&0b1)
                    fast_ch.append(fast[3 * i + j]);
                else//add NaN
                    fast_ch.append(std::numeric_limits<double>::quiet_NaN());
                if((flags[i]>>(5+j))&0b1)
                    precise_ch.append(precise[3 * i + j]);
                else//add NaN
                    precise_ch.append(std::numeric_limits<double>::quiet_NaN());
            }
            fast_list.append(fast_ch);
            precise_list.append(precise_ch);
        }
        return py::make_tuple(timestamp, fast_list, precise_list);
    }

    void
    open_connection(const char *path, int baudrate)
    {
        controller.open_connection(path, baudrate, O_RDWR | O_NOCTTY);
    }

    void
    setRGB(int id_module, int id_led, py::list rgb)
    {
        controller.setRGB(id_module, id_led, rgb[0].cast<uint8_t>(),
                          rgb[1].cast<uint8_t>(), rgb[2].cast<uint8_t>());
    }

    int
    nbModules()
    {
        return this->nb_modules;
    }
};
} // namespace ClvHd

PYBIND11_MODULE(pyclvhd, m)
{
    py::class_<ClvHd::pyDevice>(m, "Device")
        .def(py::init<int>(), py::arg("verbose") = -1)
        .def("setup", &ClvHd::pyDevice::setup, "Setup the device")
        .def("nbModules", &ClvHd::pyDevice::nbModules,
             "Get the number of modules")
        .def("open_connection", &ClvHd::pyDevice::open_connection,
             py::arg("path"), py::arg("baudrate") = 500000,
             "Open the serial connection between the computer and the "
             "controller board")
        .def("setRGB", &ClvHd::pyDevice::setRGB,
             py::arg("id_module"), py::arg("id_led"), py::arg("rgb"),
             "Set the RGB color of the given LED of the given module")
        .def("setupADS1293", &ClvHd::pyDevice::setupADS1293,
             py::arg("route_table"), py::arg("chx_enable"), py::arg("chx_high_res"),
             py::arg("chx_high_freq"), py::arg("R1"), py::arg("R2"), py::arg("R3"),
             "Setup the ADS1293 EMG modules in the device")
        .def("start_acquisition", &ClvHd::pyDevice::start_acquisition,
             "Start the acquisition of the EMG modules")
        .def("read_all", &ClvHd::pyDevice::read_all, "Read all the EMG data");
}

#endif /* __PY_CLV_HD_HPP__ */