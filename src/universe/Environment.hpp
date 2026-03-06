#pragma once
#include "../tools/Units.hpp"
#include "PhysicConfig.hpp"
#include <mutex>
#include <vector>

namespace phys
{
// Predeclerations
struct Environment;
class EnvironmentActive;

struct Body
{
    vec3d pos{};
    vec3d prev_pos{};
    vec3d vel{};
    double mass{1.0};
    uint16_t id{0};
    bool is_locked{false};
};

using Bodies = std::vector<Body>;

struct Property
{
    Color color{1.0f, 0, 0, 1.0f};
    vec3d size{1.0, 1.0, 1.0};
};

using Properties = std::vector<Property>;

struct EnvironmentBase
{
    std::vector<Body> bodies;
    double passed_time{0.0};

    EnvironmentBase() = default;
    explicit EnvironmentBase(const Environment &env);
    explicit EnvironmentBase(const EnvironmentActive &envActive);
};

struct Environment : public EnvironmentBase
{
    std::vector<Property> properties;
    UniverseConfig config;

    Environment() = default;
    Environment(const EnvironmentBase &env, std::vector<Property> properties, UniverseConfig config);
    explicit Environment(const EnvironmentActive &envActive);

    uint16_t next_id{1};
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
    std::vector<Property> &getProperties_ref();

  private:
    Environment env;
    mutable std::mutex mtx;
    friend Environment;
    friend EnvironmentBase;
};

} // namespace phys
