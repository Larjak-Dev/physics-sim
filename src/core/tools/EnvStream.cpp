#include "EnvStream.hpp"
#include "core/Environment.hpp"
#include "core/PhysicConfig.hpp"
#include "core/universe/Property.hpp"
#include <expected>
#include <filesystem>
#include <fstream>
#include <ranges>
using namespace phys;
using json = nlohmann::json;

nlohmann::json jsonVec3(const vec3d &vec)
{
    json j;
    j[0] = vec.x;
    j[1] = vec.y;
    j[2] = vec.z;
    return j;
}

vec3d dejsonVec3(const nlohmann::json &json)
{
    vec3d v;
    v.x = json[0];
    v.y = json[1];
    v.z = json[2];
    return v;
}

nlohmann::json jsonBody(const Body &body)
{
    json j;
    j["id"] = body.id;
    j["mass"] = body.mass;
    j["radius"] = body.radius;
    j["is_locked"] = body.is_locked;
    j["elastic_factor"] = body.elastic_factor;
    j["pos"] = jsonVec3(body.pos);
    j["prev_pos"] = jsonVec3(body.prev_pos);
    j["vel"] = jsonVec3(body.vel);
    j["force_additional"] = jsonVec3(body.force_additional);
    j["force_currently"] = jsonVec3(body.force_currently);
    return j;
}

Body dejsonBody(const nlohmann::json &json)
{
    Body b;
    b.id = json["id"];
    b.mass = json["mass"];
    b.radius = json["radius"];
    b.is_locked = json["is_locked"];
    b.elastic_factor = json["elastic_factor"];
    b.pos = dejsonVec3(json["pos"]);
    b.prev_pos = dejsonVec3(json["prev_pos"]);
    b.vel = dejsonVec3(json["vel"]);
    b.force_additional = dejsonVec3(json["force_additional"]);
    b.force_currently = dejsonVec3(json["force_currently"]);
    return b;
}

nlohmann::json EnvStream::EnvToJSON(const phys::EnvironmentBase &env)
{
    json j;
    j["passed_time"] = env.passed_time;
    for (auto &&[index, body] : std::views::enumerate(env.bodies))
    {
        j["bodies"][index] = jsonBody(body);
    }
    return j;
}

phys::EnvironmentBase EnvStream::JSONToEnv(const nlohmann::json &j)
{
    phys::EnvironmentBase env;
    env.passed_time = j["passed_time"];
    for (auto body : j["bodies"])
    {
        env.bodies.emplace_back(dejsonBody(body));
    }
    return env;
}

nlohmann::json EnvStream::CameraToJSON(const Camera &camera)
{
    json j;
    j["center"] = jsonVec3(camera.center);
    j["distance"] = camera.distance;
    j["x_angle"] = camera.x_angle;
    j["z_angle"] = camera.z_angle;
    return j;
}

void EnvStream::applyJSONToCamera(const nlohmann::json &j, Camera &cam)
{
    cam.center = dejsonVec3(j["center"]);
    cam.distance = j["distance"];
    cam.x_angle = j["x_angle"];
    cam.z_angle = j["z_angle"];
}

nlohmann::json EnvStream::PhysicConfigToJSON(const PhysicConfig &conf)
{
    json j;
    j["g"] = conf.force_config.freefall_config.g;
    j["G"] = conf.force_config.newtonian_config.G;
    j["force_type"] = conf.force_config.force_type;
    j["use_compiled_templates"] = conf.force_config.use_compiled_templates;
    j["step_type"] = conf.step_config.step_type;
    j["delta_time"] = conf.step_config.delta_time;
    j["total_time"] = conf.step_config.total_time;
    j["step_type"] = conf.step_config.step_type;
    j["speed"] = conf.step_config.speed;
    return j;
}

PhysicConfig EnvStream::JSONToPhysicConfig(nlohmann::json j)
{
    PhysicConfig conf;
    conf.force_config.freefall_config.g = j["g"];
    conf.force_config.newtonian_config.G = j["G"];
    conf.force_config.force_type = j["force_type"];
    conf.force_config.use_compiled_templates = j["use_compiled_templates"];
    conf.step_config.step_type = j["step_type"];
    conf.step_config.delta_time = j["delta_time"];
    conf.step_config.total_time = j["total_time"];
    conf.step_config.step_type = j["step_type"];
    conf.step_config.speed = j["speed"];
    return conf;
}

nlohmann::json EnvStream::PropertiesToJSON(const std::vector<Property> &properties)
{
    json j;
    for (auto &&[index, prop] : std::views::enumerate(properties))
    {
        auto &jj = j[index];
        jj["r"] = prop.color.r;
        jj["g"] = prop.color.g;
        jj["b"] = prop.color.b;
        jj["a"] = prop.color.a;
        jj["brightness"] = prop.brightness;
        jj["name"] = prop.name;
        jj["rotation_start"] = prop.rotation_start;
        jj["rotation_velocity"] = prop.rotation_velocity;
        jj["size"] = jsonVec3(prop.size);
        jj["tilt"] = prop.tilt;
    }
    return j;
}

std::vector<Property> EnvStream::JSONToProperties(const nlohmann::json &j)
{
    std::vector<Property> props;
    for (auto jj : j)
    {
        Property prop;
        prop.color.r = jj["r"];
        prop.color.g = jj["g"];
        prop.color.b = jj["b"];
        prop.color.a = jj["a"];
        prop.brightness = jj["brightness"];
        prop.name = jj["name"];
        prop.rotation_start = jj["rotation_start"];
        prop.rotation_velocity = jj["rotation_velocity"];
        prop.size = dejsonVec3(jj["size"]);
        prop.tilt = jj["tilt"];
        props.emplace_back(prop);
    }
    return props;
}

nlohmann::json EnvStream::UniverseToJSON(const Universe &uni)
{
    json j;
    j["env"] = this->EnvToJSON(uni.env->getEnvironment_safe());
    j["cam"] = this->CameraToJSON(*uni.camera);
    j["physic_config"] = this->PhysicConfigToJSON(uni.physicConfig);
    j["properties"] = this->PropertiesToJSON(uni.properties);
    return j;
}

void EnvStream::ApplyJSONToUniverse(const nlohmann::json &j, Universe &uni)
{
    uni.env->setEnvironment_safe(this->JSONToEnv(j["env"]));
    this->applyJSONToCamera(j["cam"], *uni.camera);
    uni.physicConfig = this->JSONToPhysicConfig(j["physic_config"]);
    uni.properties = this->JSONToProperties(j["properties"]);
}

std::expected<void, std::string> EnvStream::ExportUniToFile(const Universe &uni, std::filesystem::path file_path)
{
    if (std::filesystem::exists(file_path))
    {
        return std::unexpected<std::string>("File already exists");
    }
    std::ofstream out(file_path);

    out << this->UniverseToJSON(uni);

    out.close();
    return std::expected<void, std::string>();
}

std::expected<void, std::string> EnvStream::ImportFileToUni(Universe &uni, std::filesystem::path file_path)
{
    if (!std::filesystem::exists(file_path))
    {
        return std::unexpected<std::string>("File doesnt exist!");
    }
    json j;
    std::ifstream in(file_path);
    in >> j;
    in.close();

    ApplyJSONToUniverse(j, uni);

    return std::expected<void, std::string>();
}
