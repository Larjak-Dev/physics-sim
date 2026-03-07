#pragma once
#include "../universe/Environment.hpp"
#include "../universe/PhysicConfig.hpp"
#include "universe/Universe.hpp"

namespace phys
{

struct KinematicConfig
{
    phys::ForceType type;
    double G{6.67430e-11};
    double mass_satelite{1000};
    double mass_planet{5.972e24};
    double distance{6.771e6};
    double acceleration{9.81};

    double time_satelite{86400};
    double time_fall{10};
};

UniverseConfig createKinematicScenario(KinematicConfig config);
UniverseConfig createPerfectSatelite(double G, double mass_satelite, double mass_planet, double distance);
UniverseConfig createFreeFall(double acceleration);

phys::Universe createUniverse(const UniverseConfig config);
bool checkKinematicValidityOfUniverse(const Universe &universe, UniverseConfig config);
void prepareEnvironment(EnvironmentBase &environment, UniverseConfig config, double delta_time);

Body calcBody(UniverseConfig config, double time);

} // namespace phys
