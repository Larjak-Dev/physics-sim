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
    vec3d prev_pos{};
    vec3d vel{};
    double mass{};
    uint16_t id;
};

using Bodies = std::vector<Body>;

struct Property
{
    Color color;
    vec3d size{1.0, 1.0, 1.0};
};

using Properties = std::vector<Property>;

struct EnvironmentBase
{
    std::vector<Body> bodies;

    EnvironmentBase() = default;
    explicit EnvironmentBase(const Environment &env);
    explicit EnvironmentBase(const EnvironmentActive &envActive);
};

struct Environment : public EnvironmentBase
{
    std::vector<Property> properties;

    Environment() = default;
    Environment(const EnvironmentBase &env, std::vector<Property> properties);
    explicit Environment(const EnvironmentActive &envActive);

    void addBody(Body body, Property property);
};

///////////////////
/// Environment Active
//////////////////

class EnvironmentActive
{
  public:
    EnvironmentActive() = default;
    explicit EnvironmentActive(const EnvironmentActive &other);
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
