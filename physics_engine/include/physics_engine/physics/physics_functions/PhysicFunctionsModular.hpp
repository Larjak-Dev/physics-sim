#pragma once
#include "../../core/Environment.hpp"
#include "../../core/Units_basic.hpp"
#include "PhysicStepBuffer.hpp"
#include <functional>

namespace phys
{

using ForceFunction = std::function<vec3d(vec3d, const Body &, const EnvironmentBase &)>;
using AccelerationFunction = std::function<vec3d(vec3d, const Body &, const EnvironmentBase &)>;
using StepFunction = std::function<EnvironmentBase(const EnvironmentBase &, double, StepBuffer &)>;

using ForceFunc = vec3d (*)(vec3d, const Body &, const EnvironmentBase &);
using AccelFunc = vec3d (*)(vec3d, const Body &, const EnvironmentBase &);
using StepFunc = EnvironmentBase (*)(const EnvironmentBase &, double, StepBuffer &);

ForceFunction createNewtonianForceFunction(double G);
ForceFunction createFreeFallForceFunction(double g);
AccelerationFunction createAccelerationFunction(ForceFunction forceFunction);
StepFunction createImplicitEulerStepFunction(AccelerationFunction accelerationFunction);
StepFunction createVerletStepFunction(AccelerationFunction accelerationFunction);
StepFunction createRK4StepFunction(AccelerationFunction accelerationFunction);

} // namespace phys
