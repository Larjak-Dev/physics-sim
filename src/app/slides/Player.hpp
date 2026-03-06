
#pragma once
#include "../widgets/Scene.hpp"
#include "Slide.hpp"
#include "physics/Simulator.hpp"

namespace phys::app
{
class Player : public Slide
{
  public:
    Player();
    void tickContent();
    void tickRightBar();

  private:
    // Widgets
    SceneWidget review_panel;
    std::vector<UniverseWidget> scene_widgets;
    AlmagationWidget scene_widget_alm;
    std::shared_ptr<Camera> scenes_camera;

    // Simulator
    phys::Simulator simulator;

    // Recordings
    std::vector<std::shared_ptr<phys::Recording>> recordings;
    std::vector<char> selected_recordings;
    float timeline_select{0.0f};

    void multipleScenes();
    void almagationScene();
};
} // namespace phys::app
