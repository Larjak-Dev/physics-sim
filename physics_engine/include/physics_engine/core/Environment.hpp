#pragma once
#include "PhysicConfig.hpp"
#include "Units_basic.hpp"
#include <expected>
#include <mutex>
#include <optional>
#include <string>
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

//////////////////
/// Environment
//////////////////

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
    UniverseConfig config;

    Environment() = default;
    Environment(const EnvironmentBase &env, UniverseConfig config);
    explicit Environment(const EnvironmentActive &envActive);

    uint16_t next_id{1};
    void addBody(Body body);
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

    std::expected<std::pair<Body, int>, std::string> getBody(uint16_t bodyId);
    std::expected<void, std::string> setBody(uint16_t bodyId, Body body);
    void addBody(Body body);

  private:
    Environment env;
    mutable std::mutex mtx;
    friend Environment;
    friend EnvironmentBase;
};

} // namespace phys
