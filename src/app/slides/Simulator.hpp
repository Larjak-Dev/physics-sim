
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
    SceneWidget review_panel{this->context};
    SceneWidget simulator{this->context};

    phys::Simulator physic_sim;
    std::shared_ptr<phys::Universe> universe_sim;

    double sim_speed{1.0f};

    void showConfig();
};
} // namespace phys::app
