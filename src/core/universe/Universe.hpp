
#pragma once
#include "Camera.hpp"
#include "Environment.hpp"
#include "PhysicConfig.hpp"
#include <memory>

namespace phys
{

class Universe
{
  public:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<EnvironmentActive> env;
    PhysicConfig physicConfig;

    Universe();
    Universe copy() const;
};

} // namespace phys
