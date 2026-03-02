
#pragma once
#include "../tools/Units.hpp"
#include <variant>

namespace phys
{
////////////
// ForceConfig
////////////
enum ForceType
{
    Gravitational,
    Directional
};

struct GravitationalConfig
{
    double G{};
};

struct DirectionalConfig
{
    double acceleration{};
    vec3d direction{};
};

struct ForceConfig
{
    ForceType force_type;
    std::variant<GravitationalConfig, DirectionalConfig> force_config_variant;
};
/////////////
/// StepConfig
/////////////
enum StepType
{
    Euler
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
