
#pragma once
#include "App.hpp"
#include "slides/Simulator.hpp"

namespace phys
{

class PhysicApp : public App
{
  public:
    using App::App;

  protected:
    void tick() override;

  private:
    slides::Simulator simulatorSlide;
};
} // namespace phys
