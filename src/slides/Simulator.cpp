#include "Simulator.hpp"
#include "imgui.h"
#include "tools/Debug.hpp"
using namespace phys::slides;

void Simulator::tickContent()
{
    if (!this->physic_sim.isRunningSim())
    {
        ImGui::Begin("Preview", nullptr);
        this->reviewPanel.update(*this->universe);
        ImGui::End();
    }
    else
    {
        ImGui::Begin("Simulator", nullptr);
        this->simulator.update(*this->universe_sim);
        ImGui::End();
    }

    if (this->universe_sim)
    {
        auto uni = this->universe_sim->env->getEnvironment_safe();
        auto body = uni.bodies[0];
        phys::showDebugF("Body x:{},y:{},z:{}", body.pos.x, body.pos.y, body.pos.z);
    }
}

void Simulator::tickRightBar()
{
    ImGui::Begin("Control Panel");
    if (!this->physic_sim.isRunningSim() && ImGui::Button("Start"))
    {
        this->universe_sim = std::make_shared<Universe>(this->universe->copy());
        this->physic_sim.startSim(this->universe_sim);
    }
    if (this->physic_sim.isRunningSim() && ImGui::Button("Stop"))
    {
        this->physic_sim.stopSim();
    }

    if (this->physic_sim.isRunningSim())
    {
        if (!this->physic_sim.isPausedSim() && ImGui::Button("Pause"))
        {
            this->physic_sim.pauseSim();
        }
        if (this->physic_sim.isPausedSim() && ImGui::Button("Resume"))
        {
            this->physic_sim.resumeSim();
        }
    }
    ImGui::End();
}
