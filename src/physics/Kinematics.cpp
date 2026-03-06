#include "Kinematics.hpp"
#include "universe/PhysicConfig.hpp"
#include "universe/Universe.hpp"
using namespace phys;

UniverseConfig phys::createPerfectSatelite(double G, double mass_satelite, double mass_planet, double distance)
{
    const auto force = G * mass_planet * mass_satelite / std::pow(distance, 2);
    const auto accel = force / mass_satelite;

    const auto vel = std::sqrt(accel * distance);

    UniverseConfig config;
    config.force_config.force_type = ForceType::Newtonian;
    config.force_config.newtonian_config.G = G;
    config.distance_newtonian = distance;
    config.mass_1_newtonian = mass_satelite;
    config.mass_2_newtonian = mass_planet;
    config.vel_1_newtonian = vel;
    config.is_calculated = true;

    return config;
}

UniverseConfig phys::createFreeFall(double acceleration)
{
    UniverseConfig config;
    config.force_config.force_type = ForceType::FreeFall;
    config.force_config.freefall_config.g = acceleration;
    config.is_calculated = true;

    return config;
}

Universe phys::createUniverse(const UniverseConfig config)
{
    assert(config.is_calculated &&
           "Non calculated config, you must use createFreeFall or createPerfectSatelite to create your config!");
    Environment env;
    if (config.force_config.force_type == ForceType::FreeFall)
    {
        Body body_s;
        body_s.is_locked = false;
        body_s.pos = {0.0, 0.0, 0.0};
        body_s.mass = 1.0;
        body_s.prev_pos = {0.0, 0.0, 0.0};
        body_s.vel = {0.0, 0.0, 0.0};
        Property property_s;
        property_s.color = Color(255, 0, 0);
        property_s.size = {0.5, 0.5, 0.5};
        env.addBody(body_s, property_s);
    }
    return env;
}

bool phys::checkKinematicValidityOfUniverse(const Universe &universe, UniverseConfig config){return }

Body satelite(double distance, double velocity, double time)
{

    const auto time_circulation = (2.0 * PI * distance) / velocity;
    const auto angle_velocity = (2.0 * PI) / time_circulation;

    const auto angle = angle_velocity * time;
    const auto x = std::cos(angle) * distance;
    const auto y = std::sin(angle) * distance;

    const auto angle_vel = angle + PI / 2.0;
    const auto x_vel = std::cos(angle_vel) * velocity;
    const auto y_vel = std::sin(angle_vel) * velocity;

    Body body;
    body.pos = {x, y, 0.0};
    body.vel = {x_vel, y_vel, 0.0};
    return body;
}

Body freefall(double acceleration, double time)
{
    const auto y = acceleration * std::pow(time, 2) / 2.0;
    const auto vel = acceleration * time;

    Body body;
    body.pos = {0.0, -y, 0.0};
    body.vel = {0.0, -vel, 0.0};
    return body;
}

Body phys::calcBody(UniverseConfig config, double time)
{
    // Free fall
    const auto acceleration = config.force_config.freefall_config.g;
    // Newtonian
    const auto distance = config.distance_newtonian;
    const auto velocity = config.vel_1_newtonian;

    switch (config.force_config.force_type)
    {
    case phys::ForceType::FreeFall:
        return freefall(acceleration, time);
    case phys::ForceType::Newtonian:
        // Perfect satelite pattern
        return satelite(distance, velocity, time);
    default:
        assert(false);
        return Body();
    }
}
