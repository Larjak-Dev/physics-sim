#include "ResourcesGl.hpp"

using namespace phys::gl;

ResourcesGl::ResourcesGl()
{
    this->sphere.bufferSphere(32);
}

std::shared_ptr<ResourcesGl> resourcesGlGlobal;

void phys::gl::setResourcesGL(std::shared_ptr<ResourcesGl> resources)
{
    resourcesGlGlobal = resources;
}
std::shared_ptr<ResourcesGl> phys::gl::getResourcesGL()
{
    return resourcesGlGlobal;
}
