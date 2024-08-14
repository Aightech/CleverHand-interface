#ifndef __PY_CLV_HD_HPP__
#define __PY_CLV_HD_HPP__

#include "clvHdADS1293EMG.hpp"
#include "clvHdDevice.hpp"
#include <boost/python.hpp>

using namespace boost::python;

namespace ClvHd
{
class pyDevice : public ClvHd::Device
{
    public:
    pyDevice(int verbose = -1)
        : ClvHd::Device(verbose), ESC::CLI(verbose, "pyClvHd-Device") {};

    void
    setupADS1293(boost::python::list _route_table,
                 boost::python::list _chx_enable,
                 boost::python::list _chx_high_res,
                 boost::python::list _chx_high_freq,
                 boost::python::list _R1,
                 int R2,
                 boost::python::list _R3)
    {
        int route_table[3][2] = {{extract<int>(_route_table[0][0]),
                                  extract<int>(_route_table[0][1])},
                                 {extract<int>(_route_table[1][0]),
                                  extract<int>(_route_table[1][1])},
                                 {extract<int>(_route_table[2][0]),
                                  extract<int>(_route_table[2][1])}};
        bool chx_enable[3] = {extract<bool>(_chx_enable[0]),
                              extract<bool>(_chx_enable[1]),
                              extract<bool>(_chx_enable[2])};
        bool chx_high_res[3] = {extract<bool>(_chx_high_res[0]),
                                extract<bool>(_chx_high_res[1]),
                                extract<bool>(_chx_high_res[2])};
        bool chx_high_freq[3] = {extract<bool>(_chx_high_freq[0]),
                                 extract<bool>(_chx_high_freq[1]),
                                 extract<bool>(_chx_high_freq[2])};
        int R1[3] = {extract<int>(_R1[0]), extract<int>(_R1[1]),
                     extract<int>(_R1[2])};
        int R3[3] = {extract<int>(_R3[0]), extract<int>(_R3[1]),
                     extract<int>(_R3[2])};
        ClvHd::EMG_ADS1293::setup(*this, chx_enable, route_table, chx_high_res,
                                  chx_high_freq, R1, R2, R3);
    }

    void
    start_acquisition()
    {
        ClvHd::EMG_ADS1293::start_acquisition(*this);
    }

    boost::python::list
    read_all()
    {
        int nb_modules = 0;
        nb_modules = ClvHd::EMG_ADS1293::nb_modules;
        std::vector<double> sample(nb_modules * 6);
        uint64_t timestamp = ClvHd::EMG_ADS1293::read_all(*this, sample.data());
        boost::python::list l;
        l.append(timestamp);
        for(int i = 0; i < nb_modules * 6; i++) l.append(sample[i]);
        return l;
    }

    void
    open_connection(const char *path, int baudrate)
    {
        controller.open_connection(path, baudrate, O_RDWR | O_NOCTTY);
    }

    void
    setRGB(int id_module, int id_led, boost::python::list rgb)
    {
        controller.setRGB(id_module, id_led, extract<uint8_t>(rgb[0]),
                          extract<uint8_t>(rgb[1]), extract<uint8_t>(rgb[2]));
    }

    int nbModules()
    {
        return  this->nb_modules;
    }
};
} // namespace ClvHd

BOOST_PYTHON_MODULE(pyclvhd)
{

    class_<ClvHd::pyDevice>("Device", init<int>(arg("verbose") = -1))
        .def("setup", &ClvHd::pyDevice::setup, "Setup the device")
        .def("nbModules", &ClvHd::pyDevice::nbModules, "Get the number of modules")
        .def("open_connection", &ClvHd::pyDevice::open_connection,
             (arg("path"), arg("baudrate") = 500000),
             "Open the serial connection between the computer and the "
             "controller board")
        .def("setRGB", &ClvHd::pyDevice::setRGB,
             (arg("id_module"), arg("id_led"), arg("rgb")),
             "Set the RGB color of the given LED of the given module")
        .def("setupADS1293", &ClvHd::pyDevice::setupADS1293,
             (arg("route_table"), arg("chx_enable"), arg("chx_high_res"),
              arg("chx_high_freq"), arg("R1"), arg("R2"), arg("R3")),
             "Setup the ADS1293 EMG modules in the device")
        .def("start_acquisition", &ClvHd::pyDevice::start_acquisition,
             "Start the acquisition of the EMG modules")
        .def("read_all", &ClvHd::pyDevice::read_all, "Read all the EMG data");
}

#endif /* __PY_CLV_HD_HPP__ */
