#include "Simulator.hpp"
#include "imgui.h"
using namespace phys::slides;

void Simulator::tickContent()
{
    ImGui::Begin("Preview", nullptr);
    this->reviewPanel.update(*this->universe);
    ImGui::End();
}

void Simulator::tickRightBar()
{
}
