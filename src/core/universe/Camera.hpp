#pragma once
#include "core/Units.hpp"

namespace phys
{

struct CameraSettings
{
    unsigned int locked_body_id{0};
    double minimum_camera_distance{0};

    bool is_fixed_body_size{false};
    double fixed_size{1.2};

    bool is_scaled_body_size{false};
    double body_scale{1.0};

    bool is_render_textures{true};
    bool is_render_stars{false};
    bool is_render_grid{true};
    bool is_render_fancy{false};
    bool is_render_perspective{false};

    float fov{90};
};

struct Camera
{
    vec3d center{};
    double distance{0.0};
    double z_angle{0.0};
    double x_angle{0.0};

    CameraSettings settings;

    Camera() = default;
    inline Camera(double distance) : distance(distance)
    {
    }
    inline bool operator==(const Camera &other) const
    {
        return center == other.center && distance == other.distance && z_angle == other.z_angle &&
               x_angle == other.x_angle;
    }
    mat4d getRotationMatrix() const;
    vec3d getEye() const;

    vec3d getCrossX() const;
    vec3d getCrossY() const;
    vec3d getCrossZ() const;
};

} // namespace phys
