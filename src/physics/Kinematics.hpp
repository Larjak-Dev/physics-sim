#pragma once
#include "core/universe/Environment.hpp"
#include "core/universe/PhysicConfig.hpp"
#include "core/universe/Universe.hpp"

namespace phys
{

struct KinematicConfig
{
    phys::ForceType type{phys::ForceType::Null};
    double G{6.67430e-11};
    double mass_satelite{1000};
    double mass_planet{5.972e24};
    double distance{6.771e6};
    double acceleration{9.81};

    double time_satelite{5600 * 2.0};
    double time_fall{5};
};

UniverseConfig createKinematicScenario(KinematicConfig config);
UniverseConfig createPerfectSatelite(double G, double mass_satelite, double mass_planet, double distance);
UniverseConfig createFreeFall(double acceleration);

phys::Universe createUniverse(const UniverseConfig config);
bool checkKinematicValidityOfUniverse(const Universe &universe, UniverseConfig config);
void prepareEnvironment(EnvironmentBase &environment, UniverseConfig config, double delta_time);

Body calcBody(UniverseConfig config, double time);

} // namespace phys
