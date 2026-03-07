#pragma once
#include "../widgets/Scene.hpp"
#include "Slide.hpp"
#include "physics/Kinematics.hpp"
#include "physics/Simulator.hpp"
namespace phys::app
{

class Editor : public Slide
{

  public:
    void tickContent();
    void tickRightBar(std::shared_ptr<Universe> &universe_main);

  private:
    SceneWidget reviewPanel;
    SceneWidget simulator;

    KinematicConfig config;
};

} // namespace phys::app
