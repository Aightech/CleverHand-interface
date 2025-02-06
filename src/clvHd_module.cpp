#include "clvHd_module.hpp"
#include "clvHd_controller.hpp"

namespace ClvHd
{

void
Module::setRGB(RGBColor &color)
{
    m_controller->setRGB(id, color);
};

} // namespace ClvHd