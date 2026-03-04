
#pragma once
#include "Slide.hpp"
#include "panels/PanelTexure.hpp"

namespace phys::slides
{
class Simulator : public Slide
{
  public:
    void tickContent();
    void tickRightBar();

  private:
    panels::PanelScene reviewPanel;

    panels::PanelScene simulator;
};
} // namespace phys::slides
