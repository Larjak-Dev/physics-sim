#include "AppResources.hpp"
using namespace phys;

GlResources::GlResources()
{
    this->sphere.bufferSphere(32);
    this->grid.bufferLines(this->grid_amount, this->grid_amount, 0);
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
