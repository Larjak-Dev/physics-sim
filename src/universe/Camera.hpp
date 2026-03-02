#pragma once
#include "../tools/Units.hpp"

namespace phys
{

class Camera
{
  public:
    vec3d center;
    double distance{10.0};
    double z_angle{0.0};
    double x_angle{0.0};
};

} // namespace phys
