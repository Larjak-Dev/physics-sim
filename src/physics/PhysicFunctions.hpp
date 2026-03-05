#pragma once
#include "../universe/Environment.hpp"
#include "../universe/PhysicConfig.hpp"
#include <functional>

namespace phys
{

using ForceFunction = std::function<vec3d(vec3d, const Body &, const EnvironmentBase &)>;
using AccelerationFunction = std::function<vec3d(vec3d, const Body &, const EnvironmentBase &)>;
using StepFunction = std::function<EnvironmentBase(const EnvironmentBase &, double)>;

class PhysicFunctions
{
  public:
    ForceFunction force;
    AccelerationFunction acceleration;
    StepFunction step;

    PhysicFunctions(PhysicConfig config);
    EnvironmentBase &&stepEnv(const EnvironmentBase &env, double delta_time);
};
} // namespace phys
