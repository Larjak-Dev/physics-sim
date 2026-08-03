#pragma once
#include "app/AppResources.hpp"
#include "app/slides/Slide.hpp"
#include "app/widgets/Scene.hpp"
#include "physics/Kinematics.hpp"
#include "physics/PlanetAPI.hpp"
#include "physics/Simulator.hpp"
namespace phys::app
{

enum class PresetType
{
    Kinematic,
    SolarSystem,
    IO
};

class Editor : public Slide
{

  public:
    Editor(AppContext &context);
    void tickContent();
    void tickRightBar(std::shared_ptr<Universe> &universe_main);

  private:
    SceneWidget review_panel{this->context};
    SceneWidget simulator_panel{this->context};

    PresetType universe_type;
    KinematicConfig kinematic_config;
    PlanetAPI planet_api{this->context};

    void tickKinematic(std::shared_ptr<Universe> &universe_main);
    void tickSolarSystem(std::shared_ptr<Universe> &universe_main);
    void tickImportOutput(std::shared_ptr<Universe> &universe_main);
};

} // namespace phys::app
