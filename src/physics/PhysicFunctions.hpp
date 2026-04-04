#pragma once
#include "core/Units.hpp"
#include "core/universe/Environment.hpp"
#include "core/universe/PhysicConfig.hpp"
#include "physics/PhysicsStepBuffer.hpp"
#include <functional>

namespace phys
{

using ForceFunction = std::function<vec3d(vec3d, const Body &, const EnvironmentBase &)>;
using AccelerationFunction = std::function<vec3d(vec3d, const Body &, const EnvironmentBase &)>;
using StepFunction = std::function<EnvironmentBase(const EnvironmentBase &, double, StepBuffer &)>;

class PhysicFunctions
{
  public:
    ForceFunction force;
    AccelerationFunction acceleration;
    StepFunction step;

    PhysicFunctions(PhysicConfig config);
};

} // namespace phys
