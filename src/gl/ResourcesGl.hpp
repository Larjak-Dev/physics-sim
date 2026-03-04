#pragma once
#include "GladWrap.hpp"
#include <memory>
namespace phys::gl
{

class ResourcesGl
{
  public:
    static constexpr int gridAmount = 26;
    gl::Shader mainShader{"assets/shader.vert", "assets/shader.frag"};
    gl::VertexArray sphere{};
    gl::VertexArray grid{};

    ResourcesGl();
};

void setResourcesGL(std::shared_ptr<ResourcesGl> resources);
std::shared_ptr<ResourcesGl> getResourcesGL();

} // namespace phys::gl
