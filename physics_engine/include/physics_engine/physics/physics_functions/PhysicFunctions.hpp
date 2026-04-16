#pragma once
#include "../../core/Environment.hpp"
#include "PhysicFunctionsModular.hpp"

namespace phys
{

class PhysicFunctions
{
  public:
    PhysicFunctions(PhysicConfig config);
    vec3d force(vec3d pos, const Body &self, const EnvironmentBase &env) const;
    vec3d acceleration(vec3d pos, const Body &self, const EnvironmentBase &env) const;
    EnvironmentBase step(const EnvironmentBase &env, double dt, StepBuffer &buffer) const;

  private:
    PhysicConfig config;

    ForceFunction force_modular;
    AccelerationFunction acceleration_modular;
    StepFunction step_modular;

    ForceFunc force_template;
    AccelFunc accelaration_template;
    StepFunc step_template;

    template <auto forceFunc> friend void initFuncs(PhysicFunctions &this_target);
};

} // namespace phys
