#pragma once
#include "../widgets/Scene.hpp"
#include "Slide.hpp"
#include "physics/Kinematics.hpp"
#include "physics/PlanetAPI.hpp"
#include "physics/Simulator.hpp"
namespace phys::app
{

enum class PresetType
{
    Kinematic,
    SolarSystem
};

class Editor : public Slide
{

  public:
    void tickContent();
    void tickRightBar(std::shared_ptr<Universe> &universe_main);

  private:
    SceneWidget reviewPanel;
    SceneWidget simulator;

    PresetType universe_type;
    KinematicConfig kinematic_config;
    PlanetAPI planet_api;

    void tickKinematic(std::shared_ptr<Universe> &universe_main);
    void tickSolarSystem(std::shared_ptr<Universe> &universe_main);
};

} // namespace phys::app
