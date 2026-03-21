#pragma once
#include "../gl/GladWrap.hpp"
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

    void prepare(double deltaTime);
};

using Bodies = std::vector<Body>;

struct Property
{
    Color color{1.0f, 0, 0, 1.0f};
    vec3d size{1.0, 1.0, 1.0};
    gl::Texture *texture{nullptr};
    float brightness{0.0};
    std::string name{"Unknown"};
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

    std::pair<Body, Property> getBody(uint16_t bodyId);
    bool setBody(uint16_t bodyId, std::pair<Body, Property> body);
    void addBody(std::pair<Body, Property> body);

  private:
    Environment env;
    mutable std::mutex mtx;
    friend Environment;
    friend EnvironmentBase;
};

} // namespace phys
