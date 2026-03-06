#pragma once
#include "../universe/Environment.hpp"
#include "../universe/PhysicConfig.hpp"

namespace phys
{

UniverseConfig createPerfectSatelite(double G, double mass_satelite, double mass_planet, double distance);
UniverseConfig createFreeFall(double acceleration);
Environment createEnvironment(const UniverseConfig config);
bool checkKinematicValidityOfEnvironment(const Environment &env, UniverseConfig config);

Body calcBody(UniverseConfig config, double time);

} // namespace phys
