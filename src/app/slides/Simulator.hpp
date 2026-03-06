
#pragma once
#include "../widgets/Scene.hpp"
#include "Slide.hpp"
#include "physics/Simulator.hpp"

namespace phys::app
{
class Simulator : public Slide
{
  public:
    void tickContent();
    void tickRightBar();

  private:
    SceneWidget reviewPanel;
    SceneWidget simulator;

    phys::Simulator physic_sim;
    std::shared_ptr<phys::Universe> universe_sim;

    void showConfig();
};
} // namespace phys::app
