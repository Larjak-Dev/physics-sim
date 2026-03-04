
#pragma once
#include "App.hpp"
#include "slides/Simulator.hpp"

namespace phys
{

class PhysicApp : public App
{
  public:
    PhysicApp(sf::ContextSettings settings);

  protected:
    void tick() override;

  private:
    std::shared_ptr<Universe> universe;
    slides::Simulator simulatorSlide;
};
} // namespace phys
