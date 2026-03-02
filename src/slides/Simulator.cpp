#include "Simulator.hpp"
#include "imgui.h"
#include "panels/PanelRender.hpp"
using namespace phys::slides;

void Simulator::tickContent()
{
    ImGui::Begin("Preview");
    this->review.update();
    ImGui::End();
}

void Simulator::tickRightBar()
{
}
