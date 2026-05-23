#pragma once
#include "core/PhysicConfig.hpp"
#include "core/universe/Universe.hpp"
#include "physics/KinematicConstants.hpp"

namespace phys
{

struct KinematicConfig
{
    phys::ForceType type{constants::ENVIRONMENT_TYPE};
    double mass_satelite{constants::MASS_SATELITE};
    double mass_planet{constants::MASS_PLANET};
    double distance{constants::DISTANCE_SATELITE};

    bool use_templated_physicfunctions{true}; // Will disable custom G/acceleration modifications. If you want to change
                                              // template G/acceleration constants check KinematicConstants.hpp
    double G{constants::GRAVITY_CONSTANT};
    double acceleration{constants::ACCELERATION};

    double time_satelite{constants::TIME_SATELITE};
    double time_fall{constants::TIME_FALL};
};

UniverseConfig createKinematicScenario(KinematicConfig config);
UniverseConfig createPerfectSatelite(double G, double mass_satelite, double mass_planet, double distance);
UniverseConfig createFreeFall(double acceleration);

phys::Universe createUniverse(const UniverseConfig config);
bool checkKinematicValidityOfUniverse(const Universe &universe, UniverseConfig config);
void prepareBody(Body &body, double delta_time);
void prepareEnvironment(EnvironmentBase &environment, UniverseConfig config, double delta_time);

Body calcBody(UniverseConfig config, double time);

} // namespace phys
