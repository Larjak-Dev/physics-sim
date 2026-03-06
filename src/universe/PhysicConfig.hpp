
#pragma once

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

inline const char *getStepMetodStr(StepType type)
{
    switch (type)
    {
    case StepType::Null:
        return "Null";
        break;

    case StepType::ImplicitEuler:
        return "Implicit Euler";
        break;
    case StepType::Verlet:
        return "Verlet";
        break;
    case StepType::RK4:
        return "RK4";
        break;
    }
}

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

////////////
/// Universe Config
////////////

struct UniverseConfig
{
    bool is_calculated{false};
    ForceConfig force_config;

    // Newtonian
    double distance_newtonian{1.0};
    double mass_1_newtonian{1.0};
    double mass_2_newtonian{1.0};
    double vel_1_newtonian{1.0};
};

} // namespace phys
