#pragma once
#include "../universe/Environment.hpp"
#include "../universe/PhysicConfig.hpp"
#include <functional>

namespace phys
{

struct Derivates
{
    vec3d l{};
    vec3d k{};
};

// Buffer for memory during
struct StepBuffer
{
    std::vector<Derivates> der_1;
    std::vector<Derivates> der_2;
    std::vector<Derivates> der_3;
    std::vector<Derivates> der_4;
    std::size_t size{0};
    void buffer(std::size_t size);
};

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
