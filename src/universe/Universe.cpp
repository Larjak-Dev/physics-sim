#include "Universe.hpp"
#include "universe/Environment.hpp"

using namespace phys;

Universe::Universe()
{
    this->env = std::make_shared<EnvironmentActive>();
    this->camera = std::make_shared<Camera>();
}

Universe Universe::copy() const
{
    Universe copy = *this;
    copy.env = std::make_shared<EnvironmentActive>(*this->env);
    copy.camera = std::make_shared<Camera>(*this->camera);
    return copy;
}
