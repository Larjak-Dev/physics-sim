
#pragma once
#include "Slide.hpp"
#include "panels/PanelTexure.hpp"
#include "physics/Simulator.hpp"

namespace phys::slides
{
class Player : public Slide
{
  public:
    Player();
    void tickContent();
    void tickRightBar();

  private:
    panels::PanelScene reviewPanel;
    panels::PanelScene simulator;

    phys::Simulator physic_sim;
    std::shared_ptr<phys::Universe> universe_sim;
    std::shared_ptr<phys::Recording> recording;
    float timeline_select{0.0f};
};
} // namespace phys::slides
