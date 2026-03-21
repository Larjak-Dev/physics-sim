#pragma once
#include "GladWrap.hpp"
#include <memory>
namespace phys::gl
{

class ResourcesGl
{
  public:
    static constexpr int gridAmount = 200;
    gl::ShaderMain mainShader{};
    gl::ShaderBlur shader_basic{};
    gl::ShaderBlur shader_blur{};
    gl::ShaderCombine shader_combine{};

    gl::VertexArray sphere{};
    gl::VertexArray grid{};
    gl::VertexArray quad{};

    gl::Texture default_tex;

    gl::Texture sun;
    gl::Texture mercury;
    gl::Texture venus;
    gl::Texture mars;
    gl::Texture saturn;
    gl::Texture neptune;
    gl::Texture uranus;
    gl::Texture jupiter;

    gl::Texture stars;
    gl::Texture moon;
    gl::Texture earth_day;
    gl::Texture earth_clouds;

    ResourcesGl();
};

void setResourcesGL(std::shared_ptr<ResourcesGl> resources);
std::shared_ptr<ResourcesGl> getResourcesGL();

} // namespace phys::gl
