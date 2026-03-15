#include "PlanetAPI.hpp"
#include "../tools/Error.hpp"
#include <cpr/cpr.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace phys;
using json = nlohmann::json;

Universe PlanetAPI::createUniverse()
{
    std::string dir_nasa = "nasa";
    std::filesystem::create_directory(dir_nasa);
    std::string filePath =
        std::format("{}/solarsystem_earth_{}_{}_{}.json", dir_nasa, this->year, this->month, this->day);

    json data_earth;
    if (!std::filesystem::exists(filePath))
    {

        const std::string link =
            std::format("https://ssd.jpl.nasa.gov/api/"
                        "horizons.api?format=json&COMMAND='399'&OBJ_DATA='YES'&MAKE_EPHEM='YES'&EPHEM_TYPE='VECTORS'&"
                        "CENTER='500@0'&START_TIME='{}-{}-{}'&STOP_TIME='{}-{}-{}'&STEP_SIZE='1d'",
                        year, month, day, year, month, day + 1);

        std::cout << std::format("Fetching from: {}", link);
        auto r = cpr::Get(cpr::Url{link}, cpr::VerifySsl{false});
        if (r.status_code == 200)
        {

            std::cout << "Successful respons!";
            data_earth = json::parse(r.text);

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
        data_earth.parse(file_input);
    }

    std::string result = data_earth["result"];

    size_t start = result.find("$$SOE");
    size_t end = result.find("$$EOE");
    std::string data = result.substr(start, end);

    return Universe();
}
