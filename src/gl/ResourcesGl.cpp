#include "ResourcesGl.hpp"

using namespace phys::gl;

ResourcesGl::ResourcesGl()
{
    this->sphere.bufferSphere(32);
    this->grid.bufferLines(this->gridAmount, this->gridAmount, 0);
}

std::weak_ptr<ResourcesGl> resourcesGlGlobal;

void phys::gl::setResourcesGL(std::shared_ptr<ResourcesGl> resources)
{
    resourcesGlGlobal = resources;
}
std::shared_ptr<ResourcesGl> phys::gl::getResourcesGL()
{
    assert(!resourcesGlGlobal.expired() && "Resources Expired!");
    return resourcesGlGlobal.lock();
}
