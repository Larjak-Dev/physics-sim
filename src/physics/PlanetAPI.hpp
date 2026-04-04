#pragma once
#include "app/AppResources.hpp"
#include "core/universe/Universe.hpp"
#include "graphics/GladWrap.hpp"
namespace phys
{

enum class PlanetType
{
    Sun,
    Mercury,
    Venus,
    Earth,
    Mars,
    Jupiter,
    Saturn,
    Uranus,
    Neptune,
    Moon,
    Ganymede,
    Titan,
    Callisto,
    Io,
    Europa,
    Triton,
    Pluto
};

struct PlanetID
{
    std::string id;
    std::string name;
    phys::Color color;
    gl::Texture *texture;
    gl::Texture *texture_dark;
    gl::Texture *texture_atmosphere;
    gl::Texture *texture_ring;
    float brightness{0.0};
};

struct PlanetResult
{
    Body body_1;
    Body body_2; // Body in the future
    Property prop;
};

struct SolarSystemResult
{
    Universe universe_1;
    Universe universe_2;
};

class PlanetAPI
{
  public:
    double universe_scale{1.0};
    int year_1{2025}, month_1{1}, day_1{1};
    int year_2{2025}, month_2{1}, day_2{2};

    double total_days{365.0};

    PlanetAPI(AppContext &context);
    PlanetResult fetchPlanet(PlanetType planetType);
    SolarSystemResult fetchSolarSystem();

  private:
    AppContext &context;

    PlanetID getPlanetID(PlanetType planetType);
};

} // namespace phys
