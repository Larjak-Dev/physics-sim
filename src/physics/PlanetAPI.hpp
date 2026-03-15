#pragma once
#include "../universe/Universe.hpp"
namespace phys
{

class PlanetAPI
{
  public:
    int year{2025}, month{1}, day{1};
    Universe createUniverse();
};

} // namespace phys
