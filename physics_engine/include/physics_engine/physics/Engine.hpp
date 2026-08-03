#pragma once
#include "core/Environment.hpp"
#include <glm/geometric.hpp>
#include <physics/physics_functions/PhysicFunctions.hpp>
#include <ranges>

namespace phys
{

// returns true if any obj is colliding with something, the function F gets run for every two objects that collide:
// F(body_1, index_1, body_2, index_2, magnitude, body_radius_sum)
template <typename F> bool detectCollision(const phys::EnvironmentBase &env, F func, double margin)
{
    bool foundCollision{false};
    for (auto &&[i_1, body_1] : std::views::enumerate(env.bodies))
    {
        for (auto &&[i_2, body_2] : std::views::enumerate(env.bodies))
        {
            if (i_2 <= i_1 || body_1.id == body_2.id || body_1.radius <= 0 || body_2.radius <= 0)
                continue;

            auto delta = body_1.pos - body_2.pos;
            auto mag = glm::length(delta);
            auto body_radius_sum = body_1.radius + body_2.radius;

            if (mag < body_radius_sum - margin)
            {
                func(body_1, i_1, body_2, i_2, mag, body_radius_sum);
                foundCollision = true;
            }
        }
    }
    return foundCollision;
}

// detects collision between two bodies.
inline bool detectCollision(const Body &body_1, const Body &body_2, double margin)
{
    auto delta = body_1.pos - body_2.pos;
    auto mag = glm::length(delta);
    auto body_radius_sum = body_1.radius + body_2.radius;
    return mag < body_radius_sum - margin;
}

// returns true if any two objects collides in env_next but doesnt collide in env_org. It basically ignores objects that
// are already colliding with each other. the function F gets run for every two objects that collide:
// F(body_1, index_1, body_2, index_2, magnitude, body_radius_sum). Margin: the allowed space between two bodies that
// doesnt get processed as an collision. Make sure its not too small.
template <typename F>
bool detectCollisionLimited(const phys::EnvironmentBase &env_org, const phys::EnvironmentBase &env_next, F func,
                            double margin)
{
    bool foundCollision{false};
    for (auto &&[i_1, body_1] : std::views::enumerate(env_next.bodies))
    {
        for (auto &&[i_2, body_2] : std::views::enumerate(env_next.bodies))
        {
            if (i_2 <= i_1 || body_1.id == body_2.id || body_1.radius <= 0 || body_2.radius <= 0)
                continue;

            auto delta = body_1.pos - body_2.pos;
            auto mag = glm::length(delta);
            auto body_radius_sum = body_1.radius + body_2.radius;

            if (mag < body_radius_sum - margin && !detectCollision(env_org.bodies[i_1], env_org.bodies[i_2], margin))
            {
                func(body_1, i_1, body_2, i_2, mag, body_radius_sum);
                foundCollision = true;
            }
        }
    }
    return foundCollision;
}

// An engine steps/progresses an EnvironmentBase further in time.
class Engine
{
  public:
    Engine(PhysicFunctions functions);

    // Progresses env_org further in time based on dt. Returns the time progressed env.
    virtual EnvironmentBase step(const EnvironmentBase &env_org, double dt, StepBuffer &buffer, int count = 0) const;

  protected:
    PhysicFunctions funcs;
};

// An engine steps/progresses an EnvironmentBase further in time. This engine manages collisions between the cirles
class EngineCollision : public Engine
{
  public:
    using Engine::Engine;
    // Progresses env_org further in time based on dt. Returns the time progressed env.
    EnvironmentBase step(const EnvironmentBase &env, double dt, StepBuffer &buffer, int count = 0) const;
};

} // namespace phys
