
#include "Environment.hpp"
#include <vector>

using namespace phys;

EnvironmentBase::EnvironmentBase(const Environment &env)
{
    this->bodies = env.bodies;
    this->passed_time = env.passed_time;
}
EnvironmentBase::EnvironmentBase(const EnvironmentActive &envActive)
{
    std::lock_guard<std::mutex> mtxlock(envActive.mtx);
    this->bodies = envActive.env.bodies;
    this->passed_time = envActive.env.passed_time;
}

Environment::Environment(const EnvironmentBase &env, const std::vector<Property> properties)
{
    this->bodies = env.bodies;
    this->properties = properties;
    this->passed_time = env.passed_time;
}
Environment::Environment(const EnvironmentActive &envActive)
{
    std::lock_guard<std::mutex> mtxlock(envActive.mtx);
    this->bodies = envActive.env.bodies;
    this->properties = envActive.env.properties;
    this->passed_time = envActive.env.passed_time;
}

void Environment::addBody(Body body, Property property)
{
    this->bodies.push_back(body);
    this->bodies.back().id = this->next_id++;
    this->properties.push_back(property);
}

EnvironmentActive::EnvironmentActive(const EnvironmentActive &other)
{
    std::lock_guard<std::mutex> mtxlock(other.mtx);
    this->env = other.env;
}
EnvironmentActive::EnvironmentActive(const Environment &env)
{
    this->env = env;
}

void EnvironmentActive::setEnvironment_safe(const EnvironmentBase &env)
{
    std::lock_guard<std::mutex> mtxlock(this->mtx);
    this->env.bodies = env.bodies;
    this->env.passed_time = env.passed_time;
}
EnvironmentBase EnvironmentActive::getEnvironment_safe()
{
    return EnvironmentBase(*this);
}
