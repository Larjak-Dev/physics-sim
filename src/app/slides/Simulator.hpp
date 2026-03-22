
#pragma once
#include "Slide.hpp"
#include "app/AppResources.hpp"
#include "app/widgets/Scene.hpp"
#include "physics/Simulator.hpp"

namespace phys::app
{
class Simulator : public Slide
{
  public:
    Simulator(AppContext &context);
    void tickContent();
    void tickRightBar();

  private:
    SceneWidget reviewPanel{this->context};
    SceneWidget simulator{this->context};

    phys::Simulator physic_sim;
    std::shared_ptr<phys::Universe> universe_sim;

    void showConfig();
};
} // namespace phys::app
