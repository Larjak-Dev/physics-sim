
#pragma once
#include "../tools/Units.hpp"
#include <variant>

namespace phys
{
////////////
// ForceConfig
////////////
enum class ForceType
{
    Null,
    Newtonian,
    FreeFall
};

struct NewtonianConfig
{
    double G{};
};

struct FreeFallConfig
{
    double g{};
};

struct ForceConfig
{
    ForceType force_type;
    NewtonianConfig newtonian_config;
    FreeFallConfig freefall_config;
};
/////////////
/// StepConfig
/////////////
enum class StepType
{
    Null,
    ImplicitEuler,
    Verlet,
    RK4
};

struct StepConfig
{
    StepType step_type;
    double delta_time{0.01};
    double total_time{10};
};

/////////
/// PhysicsConfig
/////////
struct PhysicConfig
{
    ForceConfig force_config;
    StepConfig step_config;
};

} // namespace phys
