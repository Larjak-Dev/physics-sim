#pragma once
#include "core/PhysicConfig.hpp"
#include "core/universe/Camera.hpp"
#include "core/universe/Universe.hpp"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <vector>

namespace phys
{

class EnvStream
{
  public:
    nlohmann::json EnvToJSON(const phys::EnvironmentBase &env);
    phys::EnvironmentBase JSONToEnv(const nlohmann::json &j);

    nlohmann::json CameraToJSON(const Camera &camera);
    void applyJSONToCamera(const nlohmann::json &j, Camera &cam);

    nlohmann::json PhysicConfigToJSON(const PhysicConfig &conf);
    PhysicConfig JSONToPhysicConfig(nlohmann::json j);

    nlohmann::json PropertiesToJSON(const std::vector<Property> &properties);
    std::vector<Property> JSONToProperties(const nlohmann::json &j);

    nlohmann::json UniverseToJSON(const Universe &uni);
    void ApplyJSONToUniverse(const nlohmann::json &json, Universe &uni);

    std::expected<void, std::string> ExportUniToFile(const Universe &uni, std::filesystem::path path);
    std::expected<void, std::string> ImportFileToUni(Universe &uni, std::filesystem::path path);
};

} // namespace phys
