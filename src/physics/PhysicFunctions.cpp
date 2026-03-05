#include "PhysicFunctions.hpp"
#include "universe/Environment.hpp"
#include "universe/PhysicConfig.hpp"
#include <ranges>
#include <vector>

using namespace phys;

void StepBuffer::buffer(std::size_t size)
{
    if (size > this->size)
    {
        this->der_1.resize(size);
        this->der_2.resize(size);
        this->der_3.resize(size);
        this->der_4.resize(size);
    }
}

ForceFunction createNewtonianForceFunction(double G)
{
    return ForceFunction(
        [G](vec3d pos, const Body &self, const EnvironmentBase &env)
        {
            vec3d F_total{};
            for (Body other : env.bodies)
            {
                if (other.id == self.id)
                    continue;
                const auto r = glm::length(pos - other.pos);
                const auto F = (G * self.mass * other.mass * (pos - other.pos)) / (glm::pow(r, 3));
                F_total += F;
            }
            return F_total;
        });
}

ForceFunction createFreeFallForceFunction(double g)
{
    return ForceFunction(
        [g](vec3d pos, const Body &self, const EnvironmentBase &env)
        {
            vec3d F_total{0.0, -1.0, 0.0};
            F_total *= self.mass * g;

            return F_total;
        });
}

AccelerationFunction createAccelerationFunction(ForceFunction forceFunction)
{
    return AccelerationFunction(
        [forceFunction](vec3d pos, const Body &self, const EnvironmentBase &env)
        {
            const vec3d F = forceFunction(pos, self, env);
            return F / self.mass;
        });
}

Body implicitEulerBodyStep(const Body &body, double delta_time, const EnvironmentBase &env,
                           const AccelerationFunction &accelerationFunction)
{
    const auto a_0 = accelerationFunction(body.pos, body, env);
    const auto v_0 = body.vel;
    const auto s_0 = body.pos;

    const auto v_1 = v_0 + a_0 * delta_time;
    const auto s_1 = s_0 + v_1 * delta_time;

    Body body_new = body;
    body_new.vel = v_1;
    body_new.pos = s_1;
    return body_new;
}

StepFunction createImplicitEulerStepFunction(AccelerationFunction accelerationFunction)
{
    return StepFunction(
        [accelerationFunction](const EnvironmentBase &env, double delta_time, StepBuffer &buffer)
        {
            EnvironmentBase env_new;
            for (const Body &body : env.bodies)
            {
                env_new.bodies.emplace_back(implicitEulerBodyStep(body, delta_time, env, accelerationFunction));
            }

            env_new.passed_time = env.passed_time + delta_time;
            return env_new;
        });
}

////////////////
/// Verlet
///////////////

Body verletBodyStep(const Body &body, double delta_time, const EnvironmentBase &env,
                    const AccelerationFunction &accelerationFunction)
{

    const auto a_0 = accelerationFunction(body.pos, body, env);
    const auto s_0 = body.pos;
    const auto s_p1 = body.prev_pos;

    const auto s_1 = 2.0 * s_0 - s_p1 + a_0 * glm::pow(delta_time, 2.0);

    Body body_new = body;
    body_new.pos = s_1;
    body_new.prev_pos = s_0;
    return body_new;
}

StepFunction createVerletStepFunction(AccelerationFunction accelerationFunction)
{
    return StepFunction(
        [accelerationFunction](const EnvironmentBase &env, double delta_time, StepBuffer &buffer)
        {
            EnvironmentBase env_new;
            for (const Body &body : env.bodies)
            {
                env_new.bodies.emplace_back(verletBodyStep(body, delta_time, env, accelerationFunction));
            }

            env_new.passed_time = env.passed_time + delta_time;
            return env_new;
        });
}

//////////////////
/// RK4
/////////////////

EnvironmentBase advance(const EnvironmentBase &start, const std::vector<Derivates> &derivates, double delta_time)
{
    auto env_new = start;
    for (auto &&[index, body] : std::views::enumerate(env_new.bodies))
    {
        body.pos += derivates[index].l * delta_time;
        body.vel += derivates[index].k * delta_time;
    }
    return env_new;
}

void calcDerivates(const EnvironmentBase &env, const AccelerationFunction &accelerationFunction,
                   std::vector<Derivates> &bufferData)
{
    for (auto &&[i, body] : std::views::enumerate(env.bodies))
    {
        bufferData[i].l = body.vel;
        bufferData[i].k = accelerationFunction(body.pos, body, env);
    }
}

EnvironmentBase advanceWithSum(const EnvironmentBase &start, const std::vector<Derivates> &der_0,
                               const std::vector<Derivates> &der_1, const std::vector<Derivates> &der_2,
                               const std::vector<Derivates> &der_3, double delta_time)
{
    auto env = start;
    for (auto &&[i, body] : std::views::enumerate(env.bodies))
    {
        const auto l_1 = der_0[i].l;
        const auto l_2 = der_1[i].l;
        const auto l_3 = der_2[i].l;
        const auto l_4 = der_3[i].l;
        const auto k_1 = der_0[i].k;
        const auto k_2 = der_1[i].k;
        const auto k_3 = der_2[i].k;
        const auto k_4 = der_3[i].k;

        body.pos += delta_time * (l_1 + 2.0 * l_2 + 2.0 * l_3 + l_4) / 6.0;
        body.vel += delta_time * (k_1 + 2.0 * k_2 + 2.0 * k_3 + k_4) / 6.0;
    }
    return env;
}

StepFunction createRK4StepFunction(AccelerationFunction accelerationFunction)
{
    return StepFunction(
        [accelerationFunction](const EnvironmentBase &env, double delta_time, StepBuffer &buffer)
        {
            buffer.buffer(env.bodies.size());
            auto &derivates_1 = buffer.der_1;
            auto &derivates_2 = buffer.der_2;
            auto &derivates_3 = buffer.der_3;
            auto &derivates_4 = buffer.der_4;

            calcDerivates(env, accelerationFunction, derivates_1);

            const auto env_mid2 = advance(env, derivates_1, delta_time / 2.0);
            calcDerivates(env_mid2, accelerationFunction, derivates_2);

            const auto env_mid3 = advance(env, derivates_2, delta_time / 2.0);
            calcDerivates(env_mid3, accelerationFunction, derivates_3);

            const auto env_mid4 = advance(env, derivates_3, delta_time);
            calcDerivates(env_mid4, accelerationFunction, derivates_4);

            auto env_new = advanceWithSum(env, derivates_1, derivates_2, derivates_3, derivates_4, delta_time);
            env_new.passed_time += delta_time;
            return env_new;
        });
}

// Config

PhysicFunctions::PhysicFunctions(PhysicConfig config)
{

    switch (config.force_config.force_type)
    {
    case phys::ForceType::FreeFall:
        this->force = createFreeFallForceFunction(config.force_config.freefall_config.g);
        break;
    case phys::ForceType::Newtonian:
        this->force = createNewtonianForceFunction(config.force_config.newtonian_config.G);
    default:
        assert(false && "Unvalid physics config!");
        break;
    }

    this->acceleration = createAccelerationFunction(this->force);

    switch (config.step_config.step_type)
    {
    case phys::StepType::ImplicitEuler:
        this->step = createImplicitEulerStepFunction(this->acceleration);
        break;
    case phys::StepType::Verlet:
        this->step = createVerletStepFunction(this->acceleration);
        break;
    case phys::StepType::RK4:
        this->step = createRK4StepFunction(this->acceleration);
        break;
    default:
        assert(false && "Unvalid physics config!");
        break;
    }
}
