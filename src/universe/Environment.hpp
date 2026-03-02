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
    vec3d pos{};
    vec3d vel{};
    double mass{};
};

using Bodies = std::vector<Body>;

struct Property
{
    Color color;
    double radius;
};

using Properties = std::vector<Property>;

class EnvironmentBase
{
  public:
    std::vector<Body> bodies;

    EnvironmentBase() = default;
    EnvironmentBase(const Environment &env);
    EnvironmentBase(const EnvironmentActive &envActive);
};

class Environment : public EnvironmentBase
{
  public:
    std::vector<Property> properties;

    Environment() = default;
    Environment(const EnvironmentBase &env, std::vector<Property> properties);
    Environment(const EnvironmentActive &envActive);

    void addBody(Body body, Property property);
};

///////////////////
/// Environment Active
//////////////////

class EnvironmentActive
{
  public:
    EnvironmentActive() = default;
    EnvironmentActive(const EnvironmentActive &other);
    EnvironmentActive(const Environment &env);

    void setEnvironment_safe(const EnvironmentBase &env);
    EnvironmentBase getEnvironment_safe();

  private:
    Environment env;
    mutable std::mutex mtx;
    friend Environment;
    friend EnvironmentBase;
};

} // namespace phys
