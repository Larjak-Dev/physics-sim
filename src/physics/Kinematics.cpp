#include "Kinematics.hpp"
#include "universe/Environment.hpp"
#include "universe/PhysicConfig.hpp"
#include "universe/Universe.hpp"
using namespace phys;

UniverseConfig phys::createKinematicScenario(KinematicConfig config)
{
    switch (config.type)
    {
    case phys::ForceType::FreeFall:
    {
        auto uni_config = createFreeFall(config.acceleration);
        uni_config.total_time = config.time_fall;
        return uni_config;
        break;
    }
    case phys::ForceType::Newtonian:
    {
        auto uni_config = createPerfectSatelite(config.G, config.mass_satelite, config.mass_planet, config.distance);
        uni_config.total_time = config.time_satelite;
        return uni_config;
        break;
    }
    default:
        assert(false);
    }
}

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

    Universe universe;
    Environment env;
    env.config = config;
    Camera camera;
    switch (config.force_config.force_type)
    {
    case phys::ForceType::FreeFall:
    {
        Body body_s;
        body_s.is_locked = false;
        body_s.pos = {0.0, 0.0, 0.0};
        body_s.mass = 1.0;
        body_s.prev_pos = {0.0, 0.0, 0.0};
        body_s.vel = {0.0, 0.0, 0.0};
        Property property_s;
        property_s.color = Color(255, 0, 0, 255);
        property_s.size = {0.5, 0.5, 0.5};
        env.addBody(body_s, property_s);

        universe.physicConfig.force_config.force_type = ForceType::FreeFall;
        universe.physicConfig.force_config.freefall_config.g = config.force_config.freefall_config.g;

        camera.center = {0.0, 0.0, 0.0};
        camera.distance = 8;
    }
    break;
    case phys::ForceType::Newtonian:
    {
        const auto distance = config.distance_newtonian;
        const auto vel_sat = config.vel_1_newtonian;
        const auto mass_sat = config.mass_1_newtonian;
        const auto mass_planet = config.mass_2_newtonian;

        Body body_sat;
        body_sat.pos = {distance, 0.0, 0.0};
        body_sat.mass = mass_sat;
        body_sat.vel = {0.0, vel_sat, 0.0};
        body_sat.is_locked = false;

        Body body_planet;
        body_planet.pos = {0.0, 0.0, 0.0};
        body_planet.mass = mass_planet;
        body_planet.vel = {0.0, 0.0, 0.0};
        body_planet.is_locked = true;

        Property property_sat;
        property_sat.color = Color(0.6, 0.6, 0.6);
        property_sat.size = {distance * 0.2, distance * 0.2, distance * 0.2};

        Property property_planet;
        property_planet.color = Color(0.2, 0.6, 0.2);
        property_planet.size = {distance * 0.4, distance * 0.4, distance * 0.4};

        env.addBody(body_sat, property_sat);

        env.addBody(body_planet, property_planet);

        universe.physicConfig.force_config.force_type = ForceType::Newtonian;
        universe.physicConfig.force_config.newtonian_config.G = config.force_config.newtonian_config.G;

        camera.center = {0.0, 0.0, 0.0};
        camera.distance = distance * 1.3;

        universe.physicConfig.step_config.total_time = 10.0;
    }
    break;
    default:
        assert(false && "Null?");
    }
    universe.physicConfig.step_config.total_time = config.total_time;

    universe.env = std::make_shared<EnvironmentActive>(env);
    universe.camera = std::make_shared<Camera>(camera);
    return universe;
}

#include <ranges>

bool phys::checkKinematicValidityOfUniverse(const Universe &universe, UniverseConfig config)
{
    if (!config.is_calculated)
    {
        return false;
    }

    const auto delta_time = universe.physicConfig.step_config.delta_time;

    // Config check
    if (universe.physicConfig.force_config.force_type != config.force_config.force_type)
        return false;
    if (universe.physicConfig.force_config.freefall_config.g != config.force_config.freefall_config.g)
        return false;
    if (universe.physicConfig.force_config.newtonian_config.G != config.force_config.newtonian_config.G)
        return false;

    // Environment check
    auto universeRef = std::make_shared<Universe>(createUniverse(config));
    Environment envRef = static_cast<Environment>(*universeRef->env);
    prepareEnvironment(envRef, envRef.config, delta_time);

    Environment env = static_cast<Environment>(*universe.env);
    prepareEnvironment(env, env.config, delta_time);

    if (envRef.bodies.size() != env.bodies.size())
        return false;
    for (auto &&[body, bodyRef] : std::views::zip(env.bodies, envRef.bodies))
    {
        if (body.pos != bodyRef.pos)
            return false;
        if (body.mass != bodyRef.mass)
            return false;
        if (body.vel != bodyRef.vel)
            return false;
        if (body.prev_pos != bodyRef.prev_pos)
            return false;
        if (body.is_locked != bodyRef.is_locked)
            return false;
    }
    return true;
}

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
    double y = 0.0;
    double vel = 0.0;
    if (time > 0.0)
    {
        y = acceleration * std::pow(time, 2) / 2.0;
        vel = acceleration * time;
    }

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

void phys::prepareEnvironment(EnvironmentBase &env, UniverseConfig config, double delta_time)
{
    if (config.is_calculated)
    {
        if (config.force_config.force_type == ForceType::Newtonian)
        {
            auto prev_body = calcBody(config, -delta_time);
            env.bodies[0].prev_pos = prev_body.pos;
        }
    }
    if (!config.is_calculated)
    {
        for (Body &body : env.bodies)
        {
            body.prev_pos = body.pos - body.vel * delta_time;
        }
    }
}
