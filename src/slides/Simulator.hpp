
#pragma once
#include "Slide.hpp"
#include "panels/PanelRender.hpp"

namespace phys::slides
{
class Simulator : public Slide
{
  public:
    void tickContent();
    void tickRightBar();

  private:
    panels::PanelRender review;
    panels::PanelRender simulator;
};
} // namespace phys::slides
