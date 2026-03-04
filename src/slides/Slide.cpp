#include "Slide.hpp"
using namespace phys::slides;

void Slide::setUniverse(std::shared_ptr<Universe> universe)
{
    this->universe = universe;
}
