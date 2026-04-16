#pragma once
#include "Camera.hpp"
#include "Property.hpp"
#include "core/Environment.hpp"
#include "core/PhysicConfig.hpp"
#include <expected>
#include <memory>
#include <string>
#include <vector>

namespace phys
{

class Universe
{
  public:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<EnvironmentActive> env;
    PhysicConfig physicConfig;
    std::vector<Property> properties;

    Universe();
    Universe copy() const;

    void addBody(Body body, Property property);
    std::expected<std::pair<Body, Property>, std::string> getBody(uint16_t bodyId);
    std::expected<void, std::string> setBody(uint16_t bodyId, std::pair<Body, Property> pair);
};

} // namespace phys
