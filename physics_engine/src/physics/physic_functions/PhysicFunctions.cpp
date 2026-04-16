#include "physics/physics_functions/PhysicFunctions.hpp"
#include "core/Environment.hpp"
#include "core/PhysicConfig.hpp"
#include "physics/KinematicConstants.hpp"
#include "physics/physics_functions/PhysicFunctionsModular.hpp"
#include "physics/physics_functions/PhysicFunctionsTemplated.hpp"
#include "physics/physics_functions/PhysicStepBuffer.hpp"

using namespace phys;

// Config
//

template <auto forceFunc> void phys::initFuncs(PhysicFunctions &this_target)
{
    this_target.force_template = forceFunc;
    constexpr auto &accelFunc = AccelerationFunc<forceFunc>;
    this_target.accelaration_template = accelFunc;

    switch (this_target.config.step_config.step_type)
    {
    case phys::StepType::ImplicitEuler:
        this_target.step_template = &phys::implicitEulerStepFunc<accelFunc>;
        break;
    case phys::StepType::Verlet:
        this_target.step_template = &phys::verletStepFunc<accelFunc>;
        break;
    case phys::StepType::RK4:
        this_target.step_template = &phys::RK4StepFunc<accelFunc>;
        break;
    default:
        assert(false && "Unvalid physics config!");
        break;
    }
}

PhysicFunctions::PhysicFunctions(PhysicConfig config)
{
    this->config = config;
    if (config.force_config.use_compiled_templates)
    {
        switch (config.force_config.force_type)
        {
        case phys::ForceType::FreeFall:
            initFuncs<FreeFallForceFunc<constants::ACCELERATION>>(*this);
            break;
        case phys::ForceType::Newtonian:
            initFuncs<NewtonianForceFunc<constants::GRAVITY_CONSTANT>>(*this);
            break;
        default:
            assert(false && "Unvalid physics config!");
            break;
        }
    }
    else
    {

        switch (config.force_config.force_type)
        {
        case phys::ForceType::FreeFall:
            this->force_modular = createFreeFallForceFunction(config.force_config.freefall_config.g);
            break;
        case phys::ForceType::Newtonian:
            this->force_modular = createNewtonianForceFunction(config.force_config.newtonian_config.G);
            break;
        default:
            assert(false && "Unvalid physics config!");
            break;
        }

        this->acceleration_modular = createAccelerationFunction(this->force_modular);

        switch (config.step_config.step_type)
        {
        case phys::StepType::ImplicitEuler:
            this->step_modular = createImplicitEulerStepFunction(this->acceleration_modular);
            break;
        case phys::StepType::Verlet:
            this->step_modular = createVerletStepFunction(this->acceleration_modular);
            break;
        case phys::StepType::RK4:
            this->step_modular = createRK4StepFunction(this->acceleration_modular);
            break;
        default:
            assert(false && "Unvalid physics config!");
            break;
        }
    }
}

EnvironmentBase PhysicFunctions::step(const EnvironmentBase &env, double dt, StepBuffer &buffer) const
{

    if (!this->config.force_config.use_compiled_templates)
    {
        return this->step_modular(env, dt, buffer);
    }
    else
    {
        return this->step_template(env, dt, buffer);
    }
}

vec3d PhysicFunctions::force(vec3d pos, const Body &self, const EnvironmentBase &env) const
{

    if (!this->config.force_config.use_compiled_templates)
    {
        return this->force_modular(pos, self, env);
    }
    else
    {
        return this->force_template(pos, self, env);
    }
}
vec3d PhysicFunctions::acceleration(vec3d pos, const Body &self, const EnvironmentBase &env) const
{

    if (!this->config.force_config.use_compiled_templates)
    {
        return this->acceleration_modular(pos, self, env);
    }
    else
    {
        return this->accelaration_template(pos, self, env);
    }
}
