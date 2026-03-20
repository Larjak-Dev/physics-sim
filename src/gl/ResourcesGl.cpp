#include "ResourcesGl.hpp"

using namespace phys::gl;

ResourcesGl::ResourcesGl()
{
    this->sphere.bufferSphere(32);
    this->grid.bufferLines(this->gridAmount, this->gridAmount, 0);
    this->default_tex.createColor({1.0, 1.0, 1.0, 1.0});
    this->quad.bufferQuad();

    this->sun.loadFromImage("assets/sun.jpg");
    this->mercury.loadFromImage("assets/mercury.jpg");
    this->venus.loadFromImage("assets/venus_atmosphere.jpg");
    this->mars.loadFromImage("assets/mars.jpg");
    this->saturn.loadFromImage("assets/saturn.jpg");
    this->jupiter.loadFromImage("assets/jupiter.jpg");
    this->neptune.loadFromImage("assets/neptune.jpg");
    this->uranus.loadFromImage("assets/uranus.jpg");

    this->moon.loadFromImage("assets/moon.jpg");
    this->earth_day.loadFromImage("assets/earth_daymap.jpg");
    this->earth_clouds.loadFromImage("assets/earth_clouds.jpg");
    this->stars.loadFromImage("assets/stars_milky_way.jpg");
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
