
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
    std::vector<std::pair<std::shared_ptr<phys::Recording>, bool>> recordings;
    float timeline_float{0.0f};
    unsigned int timeline_frame_index{0};
    float timeline_slide_value{0.0f};

    void multipleScenes();
    void almagationScene();

    void buildDock(unsigned int id);

    void stepTimeline(int i);
    void saveAsExcel();

    void setFrameIndex(unsigned int i);
};
} // namespace phys::app
