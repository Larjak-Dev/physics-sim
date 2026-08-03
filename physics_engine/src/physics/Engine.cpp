#include "core/Environment.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/geometric.hpp"
#include "physics/physics_functions/PhysicFunctions.hpp"
#include <boost/math/tools/quartic_roots.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <physics/Engine.hpp>
#include <sys/types.h>
#include <unordered_set>

using namespace phys;

Engine::Engine(PhysicFunctions functions) : funcs(functions) {};
EnvironmentBase Engine::step(const EnvironmentBase &env, double dt, StepBuffer &buffer, int count) const
{
    return this->funcs.step(env, dt, buffer);
};

// Packs two integers indicies into an long
uint64_t pack(uint32_t num_1, uint32_t num_2)
{
    return (static_cast<uint64_t>(num_1) << 32) | num_2;
}

uint32_t unpack1(uint64_t packed)
{
    return static_cast<uint32_t>(packed >> 32);
}
uint32_t unpack2(uint64_t packed)
{
    return static_cast<uint32_t>(packed & 0xFFFFFFFF);
}

struct BodyPack
{
    Body &body_1;
    Body &body_2;
};

struct BodyPack_c
{
    const Body &body_1;
    const Body &body_2;
};

// Unpacks and returns two bodies according to the packed long with two indicies and env.
BodyPack unpack(uint64_t packed, EnvironmentBase &env)
{
    auto i_1 = unpack1(packed);
    auto i_2 = unpack2(packed);
    auto &body_1 = env.bodies[i_1];
    auto &body_2 = env.bodies[i_2];

    return {body_1, body_2};
}

// Unpacks and returns two bodies according to the packed long with two indicies and env.
const BodyPack_c unpack(uint64_t packed, const EnvironmentBase &env)
{
    auto i_1 = unpack1(packed);
    auto i_2 = unpack2(packed);
    auto &body_1 = env.bodies[i_1];
    auto &body_2 = env.bodies[i_2];

    return {body_1, body_2};
}

// Predicts the collision between two bodies according to their velocity and acceleration. Its not 100% accurate because
// it doesnt take account for the changes in acceleration.
double predictCollision(const Body &body_1, const Body &body_2, const EnvironmentBase &env,
                        const PhysicFunctions &funcs, double delta_time_limit)
{
    auto a_1 = body_1.is_locked ? vec3d{} : funcs.acceleration(body_1.pos, body_1, env);
    auto a_2 = body_2.is_locked ? vec3d{} : funcs.acceleration(body_2.pos, body_2, env);
    auto v_1 = body_1.is_locked ? vec3d{} : body_1.vel;
    auto v_2 = body_2.is_locked ? vec3d{} : body_2.vel;
    auto a_d = a_1 - a_2;
    auto v_d = v_1 - v_2;
    auto p_d = body_1.pos - body_2.pos;
    auto d = body_1.radius + body_2.radius;

    auto c4 = std::pow(glm::length(a_d), 2) / 4;
    auto c3 = glm::dot(v_d, a_d);
    auto c2 = glm::dot(p_d, a_d) + std::pow(glm::length(v_d), 2);
    auto c1 = glm::dot(p_d, v_d) * 2;
    auto c0 = std::pow(glm::length(p_d), 2) - std::pow(d, 2);

    auto results = boost::math::tools::quartic_roots(c4, c3, c2, c1, c0);
    auto final_result = std::numeric_limits<double>::max();

    if (glm::length(p_d) < d)
    {
        double negative_result = -std::numeric_limits<double>::max();
        for (auto result : results)
        {
            if (!std::isnan(result) && result > negative_result && result < 0)
                negative_result = result;
        }
        if (negative_result != -std::numeric_limits<double>::max())
            final_result = negative_result;
    }
    else
    {
        double positive_result = std::numeric_limits<double>::max();
        for (auto result : results)
        {
            if (!std::isnan(result) && result < positive_result && result > 0)
                positive_result = result;
        }
        final_result = positive_result;
    }

    if (final_result == std::numeric_limits<double>::max() || final_result > delta_time_limit ||
        final_result < -delta_time_limit)
    {
        double _result = std::numeric_limits<double>::max();
        for (auto result : results)
        {
            if (!std::isnan(result) && glm::abs(result) < glm::abs(_result))
                _result = result;
        }
        final_result = _result;
    }

    if (final_result != std::numeric_limits<double>::max() && final_result <= delta_time_limit * 2 &&
        final_result >= -delta_time_limit * 2)
        return final_result;
    else
        return std::numeric_limits<double>::quiet_NaN();
}

// Predicts which of the colliding body pairs are more likely to have collided first. It returns an vector of
// std::pairs with the bodies packed indicies and its likelihood of being the suspect (float from 0 to 1). The vector is
// sorted where the most likely suspect are at the beginning of the vector.
std::vector<std::pair<uint64_t, float>> predictSuspects(const EnvironmentBase &env_org, const EnvironmentBase &env_next,
                                                        std::unordered_set<uint64_t> &targeting_bodies)
{
    std::vector<std::pair<uint64_t, float>> suspect_values;
    for (auto packed : targeting_bodies)
    {
        auto bodies_org = unpack(packed, env_org);
        auto &body_1_org = bodies_org.body_1;
        auto &body_2_org = bodies_org.body_2;
        auto bodies_next = unpack(packed, env_next);
        auto &body_1_next = bodies_next.body_1;
        auto &body_2_next = bodies_next.body_2;

        const auto distance_org = glm::length(body_1_org.pos - body_2_org.pos) - body_1_org.radius - body_2_org.radius;
        const auto distance_next =
            glm::length(body_1_next.pos - body_2_next.pos) - body_1_next.radius - body_2_next.radius;
        const auto value = std::abs(distance_next) / std::abs(distance_org);
        suspect_values.emplace_back(packed, value);
    }
    std::ranges::sort(suspect_values,
                      [](const std::pair<uint64_t, float> &first, const std::pair<uint64_t, float> &second)
                      { return second.second < first.second; });
    return suspect_values;
}

struct CollisionDebug
{
    Body first;
    Body second;
    double dt;
};

EnvironmentBase EngineCollision::step(const EnvironmentBase &env_org, const double dt, StepBuffer &buffer,
                                      int count) const
{
    constexpr double required_collision_margin = 0.008;
    constexpr uint collision_detection_quality = 10;
    constexpr double relative_velocity_limit = 0.2;

    auto env_next = this->funcs.step(env_org, dt, buffer);
    auto env_working = env_org;
    auto dt_next = dt;
    std::unordered_set<uint64_t> targeting_bodies;
    std::vector<std::pair<uint64_t, float>> targeting_suspect_values;
    uint64_t collision_target = std::numeric_limits<uint64_t>::max();

    if (count > 100)
    {
        std::cout << "Max Depth Collision Detection Reached!" << std::endl;
        return env_next;
    }

    auto addTarget =
        [&targeting_bodies](const Body &b, size_t i, const Body &b2, size_t i2, double mag, double radius_sum)
    {
        auto key = pack(i, i2);
        targeting_bodies.insert(key);
    };

    auto isWithinMargin = [required_collision_margin](uint64_t id, const EnvironmentBase &env)
    {
        auto pack = unpack(id, env);
        auto &body_1 = pack.body_1;
        auto &body_2 = pack.body_2;

        auto len = glm::length(body_1.pos - body_2.pos);
        auto distance = glm::abs(glm::length(body_1.pos - body_2.pos) - body_1.radius - body_2.radius);
        return glm::abs(glm::length(body_1.pos - body_2.pos) - body_1.radius - body_2.radius) <=
               required_collision_margin / 2.0;
    };

    // First phase collision prediction, this phase uses linear algebra to predict exact solutions to colliding spheres
    while (phys::detectCollisionLimited(env_working, env_next, addTarget, required_collision_margin))
    {
        if (targeting_bodies.size() != targeting_suspect_values.size())
            targeting_suspect_values = predictSuspects(env_org, env_next, targeting_bodies);
        else
        {
            std::cout << "First Phase Collision Detection failed" << std::endl;
            break;
        }

        if (targeting_suspect_values.size() >= 3)
        {
            std::cout << "hi";
        }

        for (auto &suspects : targeting_suspect_values)
        {
            auto body_pack = unpack(suspects.first, env_working);

            ////////////// Handle collision
            // Detect if bodies are already touching each other and have similar relative velocities, which should
            // result in normal force being applied instead of an collision

            const vec3d normal_1 = glm::normalize(body_pack.body_2.pos - body_pack.body_1.pos);
            // DEBUG
            auto is_within_margin = isWithinMargin(suspects.first, env_working);
            auto vel_dif = glm::abs(glm::dot(body_pack.body_2.vel - body_pack.body_1.vel, normal_1));
            auto a = vel_dif <= relative_velocity_limit;
            // DEBUG

            auto vel_dif_on_normal = glm::dot(body_pack.body_1.vel - body_pack.body_2.vel, normal_1);
            auto body_1_vel_on_normal = glm::dot(body_pack.body_1.vel, normal_1);
            auto body_2_vel_on_normal = glm::dot(body_pack.body_2.vel, -normal_1);
            if (isWithinMargin(suspects.first, env_working) && glm::abs(vel_dif_on_normal) <= relative_velocity_limit)
            {
                const vec3d force_1 = this->funcs.force(body_pack.body_1.pos, body_pack.body_1, env_org);
                const vec3d force_2 = this->funcs.force(body_pack.body_2.pos, body_pack.body_2, env_org);

                double f_1 = glm::dot(force_1, normal_1);
                double f_2 = glm::dot(force_2, normal_1);
                if (body_pack.body_1.is_locked)
                    f_1 = -f_2;
                if (body_pack.body_2.is_locked)
                    f_2 = -f_1;

                const vec3d force_normal_1 = (f_2 - f_1) * normal_1 / 2.0;
                const vec3d force_normal_2 = -force_normal_1;

                body_pack.body_1.force_additional += force_normal_1;
                body_pack.body_2.force_additional += force_normal_2;

                if (!body_pack.body_1.is_locked &&
                    glm::length(body_pack.body_1.vel) < glm::length(body_pack.body_2.vel))
                {
                    body_pack.body_1.vel -= vel_dif_on_normal * normal_1;
                }
                else if (!body_pack.body_2.is_locked)
                {
                    body_pack.body_2.vel += vel_dif_on_normal * normal_1;
                }

                env_next = this->funcs.step(env_working, dt_next, buffer);
                continue;
            }
            //////////////

            // Do not predict new collision if an good collision has already been found.
            if (collision_target != std::numeric_limits<uint64_t>::max())
                continue;

            ////////////// Collision prediction
            dt_next = predictCollision(body_pack.body_1, body_pack.body_2, env_working, funcs, dt);
            if (std::isnan(dt_next))
                continue;
            env_next = this->funcs.step(env_working, dt_next, buffer);

            if (!phys::detectCollisionLimited(env_working, env_next, addTarget, required_collision_margin) &&
                isWithinMargin(suspects.first, env_next))
            {
                collision_target = suspects.first;
            }
            else
            {

                for (int quality_count = 1; quality_count < collision_detection_quality; quality_count++)
                {
                    auto body_pack = unpack(suspects.first, env_next);
                    double delta_dt = predictCollision(body_pack.body_1, body_pack.body_2, env_next, funcs, dt);
                    if (std::isnan(delta_dt))
                        continue;
                    dt_next += delta_dt;
                    env_next = this->funcs.step(env_working, dt_next, buffer);

                    if (!phys::detectCollisionLimited(env_working, env_next, addTarget, required_collision_margin) &&
                        isWithinMargin(suspects.first, env_next))
                    {
                        collision_target = suspects.first;
                        break;
                    }
                }
            }
        }
    }

exit:

    if (collision_target != std::numeric_limits<uint64_t>::max())
    {
        auto body_pack = unpack(collision_target, env_next);
        auto &body_1 = body_pack.body_1;
        auto &body_2 = body_pack.body_2;
        auto m_1 = body_1.mass;
        auto m_2 = body_2.mass;
        auto e = std::sqrt(body_1.elastic_factor * body_2.elastic_factor);

        if (body_1.is_locked && body_2.is_locked)
        {
            return env_next;
        }

        if ((body_1.is_locked || body_2.is_locked))
        {
            // MY OWN MATH (: /Larjak
            // Locked body variant collision
            auto &body_locked = body_1.is_locked ? body_1 : body_2;
            auto &body_free = body_1.is_locked ? body_2 : body_1;

            vec3d normal = glm::normalize(body_locked.pos - body_free.pos);
            double v = glm::dot(body_free.vel, normal);
            vec3d vel_result = body_free.vel - (v * normal * (1 + e));

            body_free.vel = vel_result;
            return this->step(env_next, dt - dt_next, buffer, count + 1);
        }
        else
        {
            // MY OWN EQUATIONS (: /Larjak
            //  Elastic
            vec3d normal_1 = glm::normalize(body_2.pos - body_1.pos);
            vec3d normal_2 = -normal_1;

            double v_1 = glm::dot(body_1.vel, normal_1);
            double v_2 = -glm::dot(body_2.vel, normal_2);

            double v_1_out = e * (v_2 * 2 * m_2 + v_1 * (m_1 - m_2)) / (m_1 + m_2);
            double v_2_out = e * -(v_1 * 2 * m_1 + v_2 * (m_2 - m_1)) / (m_1 + m_2);

            vec3d vel_1_result = body_1.vel - v_1 * normal_1 + v_1_out * normal_1;
            vec3d vel_2_result = -body_2.vel - v_2 * normal_2 + v_2_out * normal_2;

            body_1.vel = vel_1_result;
            body_2.vel = vel_2_result;

            std::cout << "dt time: " << std::to_string(dt - dt_next) << std::endl;
            return this->step(env_next, dt - dt_next, buffer, count + 1);
        }
    }

    return env_next;
}
