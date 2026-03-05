#include "PhysicFunctions.hpp"
#include "universe/Environment.hpp"
#include "universe/PhysicConfig.hpp"

using namespace phys;

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
    return StepFunction([accelerationFunction](const EnvironmentBase &env, double delta_time) { return env; });
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
    return StepFunction([accelerationFunction](const EnvironmentBase &env, double delta_time) { return env; });
}

//////////////////
/// RK4
/////////////////

struct BodyRK4
{
    vec3d pos;
    vec3d k;
    vec3d l;
};

EnvironmentBase advance(const EnvironmentBase &env, vec3d l, vec3d k, double delta_time)
{
    EnvironmentBase env_new = env;
    for (const Body &body : env_new.bodies)
    {
        body.pos = body.pos + l * delta_time;
        body.vel += k * delta_time;
    }
    return env_new;
}

StepFunction createRK4StepFunction(AccelerationFunction accelerationFunction)
{
    return StepFunction(
        [accelerationFunction](const EnvironmentBase &env, double delta_time)
        {
            EnvRK4 i1;
            for (const Body &body : env.bodies)
            {
                const auto A_func = std::function<vec3d(vec3d)>([&accelerationFunction, &body, &env](vec3d pos)
                                                                { return accelerationFunction(pos, body, env); });
                const auto v_0 = body.vel;
                const auto s_0 = body.pos;
            }
            /*
                        const auto k_1 = A_func(s_0);
                        const auto l_1 = v_0;

                        const auto k_2 = A_func(s_0 + v_0 * delta_time * l_1 / 2.0);
                        const auto l_2 = v_0 + delta_time * k_1 / 2.0;

                        const auto k_3 = A_func(s_0 + v_0 * delta_time * l_2 / 2.0);
                        const auto l_3 = v_0 + delta_time * k_2 / 2.0;

                        const auto k_4 = A_func(s_0 + v_0 * delta_time * l_3);
                        const auto l_4 = v_0 + delta_time * k_3;

                        const auto s_1 = s_0 + delta_time * (l_1 + 2.0 * l_2 + 2.0 * l_3 + l_4) / 6.0;
                        const auto v_1 = v_0 + delta_time * (k_1 + 2.0 * k_2 + 2.0 * k_3 + k_4) / 6.0;

                        Body body_new = body;
                        body_new.vel = v_1;
                        body_new.pos = s_1;
            */
            return env;
        });
}

PhysicFunctions::PhysicFunctions(PhysicConfig config)
{

    switch (config.force_config.force_type)
    {
    case phys::ForceType::FreeFall:
        this->force = createFreeFallForceFunction(config.force_config.freefall_config.g);
        break;
    case phys::ForceType::Newtonian:
        this->force = createNewtonianForceFunction(config.force_config.newtonian_config.G);
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
    }
}
EnvironmentBase &&PhysicFunctions::stepEnv(const EnvironmentBase &env, double delta_time)
{
}
