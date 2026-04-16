#include "PlanetAPI.hpp"
#include "core/PhysicConfig.hpp"
#include "core/tools/Error.hpp"
#include "graphics/GladWrap.hpp"
#include "physics/KinematicConstants.hpp"
#include <cpr/cpr.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <glm/ext/scalar_constants.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <variant>

using namespace phys;
using json = nlohmann::json;

PlanetID PlanetAPI::getPlanetID(PlanetType planetType)
{

    auto &resources_gl = this->context.resources_gl;

    std::string id;
    std::string name;
    phys::Color color;
    float brightness{0.0};
    gl::Texture *texture{nullptr};
    gl::Texture *texture_atmosphere{nullptr};
    gl::Texture *texture_dark{nullptr};
    gl::Texture *texture_ring{nullptr};

    switch (planetType)
    {
    case PlanetType::Sun:
        id = "10";
        name = "Sun";
        color = {1.0f, 1.0f, 0.4f};
        texture = &resources_gl.sun;
        brightness = 1.0;
        break;
    case PlanetType::Mercury:

        id = "199";
        name = "Mercury";
        color = {0.6f, 0.6f, 0.6f};
        texture = &resources_gl.mercury;
        break;
    case PlanetType::Venus:

        id = "299";
        name = "Venus";
        color = {0.7f, 0.7f, 0.5f};
        texture = &resources_gl.venus;
        break;
    case PlanetType::Earth:

        id = "399";
        name = "Earth";
        color = {0, 0.5f, 0};
        texture = &resources_gl.earth_day;
        texture_dark = &resources_gl.earth_night;
        texture_atmosphere = &resources_gl.earth_clouds;
        brightness = 0.2;
        break;
    case PlanetType::Mars:

        id = "499";
        name = "Mars";
        color = {0.8f, 0.3f, 0.2f};
        texture = &resources_gl.mars;
        break;
    case PlanetType::Jupiter:

        id = "599";
        name = "Jupiter";
        color = {0.0, 1.0, 1.0};
        texture = &resources_gl.jupiter;
        break;
    case PlanetType::Saturn:

        id = "699";
        name = "Saturn";
        color = {0.8f, 0.8f, 0.6f};
        texture = &resources_gl.saturn;
        texture_ring = &resources_gl.saturn_ring;
        break;
    case PlanetType::Uranus:

        id = "799";
        name = "Uranus";
        color = {0.6f, 0.8f, 0.9f};
        texture = &resources_gl.uranus;
        break;
    case PlanetType::Neptune:

        id = "899";
        name = "Neptune";
        color = {0.4f, 0.6f, 0.9f};
        texture = &resources_gl.neptune;
        break;
    case PlanetType::Moon:

        id = "301";
        name = "Moon";
        color = {0.7f, 0.7f, 0.7f};
        texture = &resources_gl.moon;
        break;
    case PlanetType::Ganymede:

        id = "503";
        name = "Ganymede";
        color = {0.6f, 0.5f, 0.4f};
        break;
    case PlanetType::Titan:

        id = "606";
        name = "Titan";
        color = {0.9f, 0.8f, 0.3f};
        break;
    case PlanetType::Callisto:

        id = "504";
        name = "Callisto";
        color = {0.4f, 0.4f, 0.4f};
        break;
    case PlanetType::Io:

        id = "501";
        name = "Io";
        color = {0.9f, 0.9f, 0.2f};
        break;
    case PlanetType::Europa:

        id = "502";
        name = "Europa";
        color = {0.8f, 0.7f, 0.6f};
        break;
    case PlanetType::Triton:

        id = "801";
        name = "Trition";
        color = {0.8f, 0.8f, 0.8f};
        break;
    case PlanetType::Pluto:

        id = "999";
        name = "Pluto";
        color = {0.7f, 0.6f, 0.5f};
        break;
    default:
        assert(false && "Unvalid PlanetType");
        break;
    }
    return PlanetID{id, name, color, texture, texture_dark, texture_atmosphere, texture_ring, brightness};
}

PlanetAPI::PlanetAPI(AppContext &context) : context(context)
{
}

PlanetResult PlanetAPI::fetchPlanet(PlanetType planetType)
{
    PlanetID planet = getPlanetID(planetType);
    const auto planet_id = planet.id;

    const bool always_create_file = false;
    const std::string dir_nasa = "nasa";
    const std::string filePath =
        std::format("{}/solarsystem_{}_{}_{}_{}_{}_{}_{}.json", dir_nasa, planet.name, this->year_1, this->month_1,
                    this->day_1, this->year_2, this->month_2, this->day_2);

    std::filesystem::create_directory(dir_nasa);

    json data_planet;
    if (!std::filesystem::exists(filePath) || always_create_file)
    {

        const std::string link =
            std::format("https://ssd.jpl.nasa.gov/api/"
                        "horizons.api?format=json&COMMAND='{}'&OBJ_DATA='YES'&MAKE_EPHEM='YES'&EPHEM_TYPE='VECTORS'&"
                        "CENTER='500@0'&START_TIME='{}-{}-{}'&STOP_TIME='{}-{}-{}'&STEP_SIZE='1d'",
                        planet.id, year_1, month_1, day_1, year_2, month_2, day_2);

        std::cout << std::format("Fetching from: {}", link);
        auto r = cpr::Get(cpr::Url{link}, cpr::VerifySsl{false});
        if (r.status_code == 200)
        {

            std::cout << "Successful respons!";
            data_planet = json::parse(r.text);

            std::ofstream file_out(filePath);
            file_out << r.text;
        }
        else
        {
            phys::showMessageF("Failed to connect to Nasa server! Respons code: {}, Error message: {}", r.status_code,
                               r.error.message);
            std::cout << "Failed respons!";
        }
    }
    else
    {
        std::ifstream file_input(filePath);
        data_planet = json::parse(file_input);
    }

    const std::string result = data_planet["result"];
    const std::regex regex_number(R"([-+]?\d*\.?\d+([eE][-+]?\d+)?)");

    /////Mass
    const size_t start_m = result.find("Mass");
    const size_t end_m = result.find("layers");
    const std::string mass_data = result.substr(start_m, end_m);

    const auto regex_m_begin = std::sregex_iterator(mass_data.begin(), mass_data.end(), regex_number);
    const auto regex_m_end = std::sregex_iterator();

    auto i_m = regex_m_begin;
    std::advance(i_m, 1);
    const auto MASS_E_MULTIPLE = *i_m;
    std::advance(i_m, 1);
    const auto MASS_E = *i_m;

    // Radius
    const size_t start_r = result.find("radius");
    const size_t end_r = result.find("layers");
    const std::string data_r = result.substr(start_r, end_r);

    const auto regex_r_begin = std::sregex_iterator(data_r.begin(), data_r.end(), regex_number);
    const auto regex_r_end = std::sregex_iterator();

    auto i_r = regex_r_begin;
    const auto RADIUS = *i_r;

    {
        int index_it = 0;
        for (auto i = regex_r_begin; i != regex_r_end; i++)
        {
            std::smatch match = *i;
            std::string match_str = match.str();
            auto print_string = std::format("{}, {}", index_it++, match_str);
            std::cout << print_string << std::endl;
        }
    }

    // Rot. Rate (rad/s)        = 0.00007292115
    size_t start_ro = result.find("Rot");
    size_t end_ro = result.find("Surface");
    if (start_ro == std::string::npos)
    {
        start_ro = result.find("rate");
        end_ro = result.find("Mean");
    }
    const std::string str_ro = result.substr(start_ro, end_ro);

    const auto regex_ro_begin = std::sregex_iterator(str_ro.begin(), str_ro.end(), regex_number);
    const auto regex_ro_end = std::sregex_iterator();

    const auto ROTATION = *regex_ro_begin;

    // Obliquity to orbit, deg  = 23.4392911
    const size_t start_tilt = result.find("Obliquity");
    const size_t end_tilt = result.find("Sidereal");
    const std::string tilt_str = result.substr(start_tilt, end_tilt);

    const auto regex_tilt_begin = std::sregex_iterator(tilt_str.begin(), tilt_str.end(), regex_number);
    const auto regex_tilt_end = std::sregex_iterator();

    const auto TILT = *regex_tilt_begin;

    /////Cordinates and Velocity
    const size_t start = result.find("$$SOE");
    const size_t end = result.find("$$EOE");
    const std::string unit_data = result.substr(start, end);

    const auto regex_begin = std::sregex_iterator(unit_data.begin(), unit_data.end(), regex_number);
    const auto regex_end = std::sregex_iterator();

    // Just learned some regex, this stuff is AWSOME!!
    {
        int index_it = 0;
        for (auto i = regex_begin; i != regex_end; i++)
        {
            std::smatch match = *i;
            std::string match_str = match.str();
            auto print_string = std::format("{}, {}", index_it++, match_str);
            std::cout << print_string << std::endl;
        }
    }

    auto i_1 = regex_begin;
    std::advance(i_1, 6);
    const auto X1 = *i_1;
    const auto Y1 = *(++i_1);
    const auto Z1 = *(++i_1);

    const auto X_V1 = *(++i_1);
    const auto Y_V1 = *(++i_1);
    const auto Z_V1 = *(++i_1);

    auto i_2 = i_1;
    std::advance(i_2, 15);
    const auto X2 = *i_2;
    const auto Y2 = *(++i_2);
    const auto Z2 = *(++i_2);

    const auto X_V2 = *(++i_2);
    const auto Y_V2 = *(++i_2);
    const auto Z_V2 = *(++i_2);

    // Values
    double x1 = std::stod(X1.str());
    double y1 = std::stod(Y1.str());
    double z1 = std::stod(Z1.str());

    double x_v1 = std::stod(X_V1.str());
    double y_v1 = std::stod(Y_V1.str());
    double z_v1 = std::stod(Z_V1.str());

    double x2 = std::stod(X2.str());
    double y2 = std::stod(Y2.str());
    double z2 = std::stod(Z2.str());

    double x_v2 = std::stod(X_V2.str());
    double y_v2 = std::stod(Y_V2.str());
    double z_v2 = std::stod(Z_V2.str());

    vec3d pos_1 = {x1, y1, z1};
    vec3d vel_1 = {x_v1, y_v1, z_v1};
    vec3d pos_2 = {x2, y2, z2};
    vec3d vel_2 = {x_v2, y_v2, z_v2};

    pos_1 *= 1000.0;
    vel_1 *= 1000.0;
    pos_2 *= 1000.0;
    vel_2 *= 1000.0;

    double mass_e = std::stod(MASS_E.str());
    double mass_e_multiple = std::stod(MASS_E_MULTIPLE.str());
    double mass = mass_e * std::pow(10.0, mass_e_multiple);

    double radius = std::stod(RADIUS.str());
    radius *= 1000.0;

    float rotation_speed = std::stof(ROTATION.str());
    float tilt = glm::pi<float>() * std::stof(TILT.str()) / 180.0f;

    // Results
    Body body_1;
    body_1.is_locked = false;
    body_1.mass = mass;
    body_1.pos = pos_1;
    body_1.vel = vel_1;

    Body body_2;
    body_2.is_locked = false;
    body_2.mass = mass;
    body_2.pos = pos_2;
    body_2.vel = vel_2;

    Property prop;
    prop.color = planet.color;
    prop.size = {radius, radius, radius};
    prop.texture = planet.texture;
    prop.texture_dark = planet.texture_dark;
    prop.texture_atmosphere = planet.texture_atmosphere;
    prop.texture_ring = planet.texture_ring;
    prop.brightness = planet.brightness;
    prop.name = planet.name;
    prop.rotation_velocity = rotation_speed;
    prop.tilt = tilt;

    return {body_1, body_2, prop};
}
SolarSystemResult PlanetAPI::fetchSolarSystem()
{
    auto moon = fetchPlanet(PlanetType::Moon);

    auto mercury = fetchPlanet(PlanetType::Mercury);
    auto venus = fetchPlanet(PlanetType::Venus);
    auto earth = fetchPlanet(PlanetType::Earth);
    auto mars = fetchPlanet(PlanetType::Mars);
    auto jupiter = fetchPlanet(PlanetType::Jupiter);
    auto saturn = fetchPlanet(PlanetType::Saturn);
    auto uranus = fetchPlanet(PlanetType::Uranus);
    auto neptune = fetchPlanet(PlanetType::Neptune);

    auto sun = fetchPlanet(PlanetType::Sun);

    SolarSystemResult result;
    auto &universe_1 = result.universe_1;
    auto &universe_2 = result.universe_2;

    universe_1.addBody(moon.body_1, moon.prop);

    universe_1.addBody(mercury.body_1, mercury.prop);
    universe_1.addBody(venus.body_1, venus.prop);
    universe_1.addBody(earth.body_1, earth.prop);
    universe_1.addBody(mars.body_1, mars.prop);
    universe_1.addBody(jupiter.body_1, jupiter.prop);
    universe_1.addBody(saturn.body_1, saturn.prop);
    universe_1.addBody(uranus.body_1, uranus.prop);
    universe_1.addBody(neptune.body_1, neptune.prop);

    universe_1.addBody(sun.body_1, sun.prop);

    universe_1.camera->distance = 2.30e11;
    universe_1.camera->settings.is_fixed_body_size = true;
    universe_1.camera->settings.fixed_size = 0.8;
    universe_1.physicConfig.force_config.force_type = ForceType::Newtonian;
    universe_1.physicConfig.force_config.newtonian_config.G = constants::GRAVITY_CONSTANT;

    universe_1.physicConfig.step_config.delta_time = 86400.0 / 4.0;
    universe_1.physicConfig.step_config.total_time = this->total_days * 86400.0;
    universe_1.physicConfig.step_config.speed = 86400.0 * 4.0;
    universe_1.physicConfig.force_config.use_compiled_templates = true;

    return result;
}
