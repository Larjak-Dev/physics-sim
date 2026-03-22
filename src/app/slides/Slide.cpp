#include "Slide.hpp"
using namespace phys::app;

Slide::Slide(AppContext &context) : context(context)
{
}

void Slide::setUniverse(std::shared_ptr<Universe> universe)
{
    this->universe = universe;
}
