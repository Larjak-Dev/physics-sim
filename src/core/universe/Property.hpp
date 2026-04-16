#pragma once
#include "../../graphics/GladWrap.hpp"
#include "../Units.hpp"
#include <string>
#include <vector>

namespace phys
{

struct Property
{
    Color color{1.0f, 0, 0, 1.0f};
    vec3d size{1.0, 1.0, 1.0};
    gl::Texture *texture{nullptr};
    gl::Texture *texture_dark{nullptr};
    gl::Texture *texture_atmosphere{nullptr};
    gl::Texture *texture_ring{nullptr};
    float brightness{0.0};
    std::string name{"Unknown"};
    float tilt{0.0f};
    float rotation_start{0.0f};
    float rotation_velocity{0.0f};
};

using Properties = std::vector<Property>;

} // namespace phys
