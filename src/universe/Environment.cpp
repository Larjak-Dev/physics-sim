
#include "Environment.hpp"
#include "universe/PhysicConfig.hpp"
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

Environment::Environment(const EnvironmentBase &env, const std::vector<Property> properties, UniverseConfig config)
{
    this->bodies = env.bodies;
    this->properties = properties;
    this->passed_time = env.passed_time;
    this->config = config;
}
Environment::Environment(const EnvironmentActive &envActive)
{
    std::lock_guard<std::mutex> mtxlock(envActive.mtx);
    this->bodies = envActive.env.bodies;
    this->properties = envActive.env.properties;
    this->passed_time = envActive.env.passed_time;
    this->config = envActive.env.config;
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

std::vector<Property> &EnvironmentActive::getProperties_ref()
{
    return this->env.properties;
}

Body EnvironmentActive::getBody(uint16_t bodyId)
{
    std::lock_guard<std::mutex> mtxlock(this->mtx);
    auto body = std::ranges::find_if(this->env.bodies, [bodyId](auto &item) { return item.id == bodyId; });
    if (body != this->env.bodies.end())
    {
        return *body;
    }
    return Body();
}

bool EnvironmentActive::setBody(uint16_t bodyId, Body body_)
{
    std::lock_guard<std::mutex> mtxlock(this->mtx);
    auto body = std::ranges::find_if(this->env.bodies, [bodyId](auto &item) { return item.id == bodyId; });
    if (body != this->env.bodies.end())
    {
        body->is_locked = body_.is_locked;
        body->mass = body_.mass;
        body->pos = body_.pos;
        body->vel = body_.vel;
        return true;
    }
    return false;
}
