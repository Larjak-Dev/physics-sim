
#pragma once
#include "App.hpp"
#include "app/AppResources.hpp"
#include "slides/Editor.hpp"
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
    AppContext appContext;
    void tick() override;

  private:
    SlideType selected_slide{SlideType::Editor};
    std::shared_ptr<Universe> universe;

    Editor editor_slide{appContext};
    Simulator simulator_slide{appContext};
    Player player_slide{appContext};

    void buildDock(int dock_id);
};
} // namespace phys::app
