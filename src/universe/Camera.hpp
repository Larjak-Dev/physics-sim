#pragma once
#include "../tools/Units.hpp"

namespace phys
{

struct Camera
{
    vec3d center{};
    double distance{0.0};
    double z_angle{0.0};
    double x_angle{0.0};

    Camera() = default;
    inline Camera(double distance) : distance(distance)
    {
    }
    inline bool operator==(const Camera &other) const = default;
    mat4d getRotationMatrix() const;
    vec3d getEye() const;

    vec3d getCrossX() const;
    vec3d getCrossY() const;
    vec3d getCrossZ() const;
};

} // namespace phys
