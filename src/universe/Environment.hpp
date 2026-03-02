#pragma once
#include "../tools/Units.hpp"
#include <mutex>
#include <vector>

namespace phys
{
// Predeclerations
class Environment;
class EnvironmentActive;

struct Body
{
    vec3w pos{};
    vec3w vel{};
    double mass{};
};

struct Property
{
    Color color;
    double radius;
};

class EnvironmentBase
{
  public:
    std::vector<Body> bodies;

    EnvironmentBase(const Environment &env);
    EnvironmentBase(const EnvironmentActive &envActive);
};

class Environment : public EnvironmentBase
{
  public:
    std::vector<Property> properties;

    Environment(const EnvironmentBase &env);
    Environment(const EnvironmentActive &envActive);

    void addBody(Body body, Property property);
};

///////////////////
/// Environment Active
//////////////////

class EnvironmentActive
{
  public:
    EnvironmentActive(const EnvironmentActive &other);
    EnvironmentActive(const Environment &env);

    void setEnvironment_safe(const EnvironmentBase &env);
    EnvironmentBase getEnvironment_safe();

  private:
    Environment env;
    std::mutex lock;
};

} // namespace phys
