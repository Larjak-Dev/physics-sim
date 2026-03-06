#include "Simulator.hpp"
#include "../widgets/extra.hpp"
#include "imgui.h"
#include "tools/Debug.hpp"
#include "universe/PhysicConfig.hpp"
#include <utility>
using namespace phys::app;

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
    // Config
    ImGui::Begin("Control Panel");

    ImGui::BeginDisabled(this->physic_sim.isRunningSim());

    static const std::vector<std::pair<phys::StepType, const char *>> methods = {
        {phys::StepType::ImplicitEuler, "Implicit Euler"},
        {phys::StepType::Verlet, "Verlet"},
        {phys::StepType::RK4, "RK4"}};
    EnumCombo("Step Method", this->universe->physicConfig.step_config.step_type, methods);

    ImGui::InputDouble("Delta Time", &this->universe->physicConfig.step_config.delta_time);

    ImGui::EndDisabled();

    // Simulator Buttons
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

void showConfig()
{
}
