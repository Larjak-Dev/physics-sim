
#pragma once
#include "App.hpp"
#include "slides/Player.hpp"
#include "slides/Simulator.hpp"

namespace phys::app
{

enum class SlideType
{
    Editor,
    Simulator,
    Player
};

class PhysicApp : public App
{
  public:
    PhysicApp(sf::ContextSettings settings);

  protected:
    void init() override;
    void tick() override;

  private:
    SlideType selectedSlide;
    std::shared_ptr<Universe> universe;
    Simulator simulatorSlide;
    Player playerSlide;

    void buildDock(int dock_id);
};
} // namespace phys::app
