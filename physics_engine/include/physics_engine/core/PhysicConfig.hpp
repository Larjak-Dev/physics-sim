
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
inline const char *getForceStr(ForceType type)
{
    switch (type)
    {
    case phys::ForceType::Null:
        return "Null";
    case phys::ForceType::FreeFall:
        return "FreeFall";
    case phys::ForceType::Newtonian:
        return "Newtonian";
    }
}

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
    ForceType force_type{ForceType::Null};
    NewtonianConfig newtonian_config;
    FreeFallConfig freefall_config;

    // Compiled Templated Config
    bool use_compiled_templates{false};
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
    StepType step_type{StepType::Null};
    double delta_time{0.01};
    double total_time{10};
    double speed{1.0};
};

/////////
/// PhysicsConfig used as a config for generating PhysicsFunctions used for simulating
/// ForceConfig, StepConfing are used to create modular realtime generated PhysicsFunctions.
/// PhysicsConfig_TemplatedTypes are set to use a precompiled templated PhysicsFunction.
/////////
struct PhysicConfig
{
    // Modular Config
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

    // Other
    double total_time{10.0};
    double speed{1.0};
};

} // namespace phys
