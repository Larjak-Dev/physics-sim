#pragma once
#include "GladWrap.hpp"
#include <memory>
namespace phys::gl
{

class ResourcesGl
{
  public:
    gl::Shader mainShader{"assets/shader.vert", "assets/shader.frag"};
    gl::VertexArray sphere{};

    ResourcesGl();
};

void setResourcesGL(std::shared_ptr<ResourcesGl> resources);
std::shared_ptr<ResourcesGl> getResourcesGL();

} // namespace phys::gl
