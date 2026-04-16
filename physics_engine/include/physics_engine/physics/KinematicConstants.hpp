#pragma once
#include "core/PhysicConfig.hpp"

namespace phys::constants
{

// FreeFall Default
inline constexpr double ACCELERATION = {9.81};

// Newtonian Default
inline constexpr double GRAVITY_CONSTANT{6.67430e-11};

// Environment Preset
inline constexpr phys::ForceType ENVIRONMENT_TYPE{phys::ForceType::Newtonian};

// Circular Satellite Default
inline constexpr double MASS_SATELITE{1000};
inline constexpr double MASS_PLANET{5.972e24};
inline constexpr double DISTANCE_SATELITE{6.771e6};

inline constexpr double TIME_SATELITE{5600 * 2.0};
inline constexpr double TIME_FALL{5};

} // namespace phys::constants
