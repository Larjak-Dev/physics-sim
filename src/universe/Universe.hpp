
#pragma once
#include "Camera.hpp"
#include "Environment.hpp"
#include "PhysicConfig.hpp"
#include "Renderer.hpp"
#include <memory>

namespace phys
{

class Universe
{
  public:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<EnvironmentActive> env;
    Renderer render_config;

    ForceConfig force_config;
    StepConfig step_config;
};

} // namespace phys
