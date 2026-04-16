#include "Universe.hpp"

using namespace phys;

Universe::Universe()
{
    this->env = std::make_shared<EnvironmentActive>();
    this->camera = std::make_shared<Camera>();
}

Universe Universe::copy() const
{
    Universe copy;
    copy.env = std::make_shared<EnvironmentActive>(*this->env);
    copy.camera = std::make_shared<Camera>(*this->camera);
    copy.physicConfig = this->physicConfig;
    copy.properties = this->properties;
    return copy;
}

void Universe::addBody(Body body, Property property)
{
    this->env->addBody(body);
    this->properties.push_back(property);
}

std::expected<std::pair<Body, Property>, std::string> Universe::getBody(uint16_t bodyId)
{
    auto result = this->env->getBody(bodyId);
    if (result)
    {
        auto [body, index] = *result;
        if (index != -1 && index < (int)this->properties.size())
        {
            return std::make_pair(body, this->properties[index]);
        }
        return std::unexpected("Property index out of bounds for body ID " + std::to_string(bodyId));
    }
    return std::unexpected(result.error());
}

std::expected<void, std::string> Universe::setBody(uint16_t bodyId, std::pair<Body, Property> pair)
{
    auto result = this->env->getBody(bodyId);
    if (result)
    {
        auto [body, index] = *result;
        if (index != -1 && index < (int)this->properties.size())
        {
            this->env->setBody(bodyId, pair.first);
            this->properties[index] = pair.second;
            return {};
        }
        return std::unexpected("Property index out of bounds for body ID " + std::to_string(bodyId));
    }
    return std::unexpected(result.error());
}
