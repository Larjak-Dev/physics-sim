#include "Simulator.hpp"
#include "app/widgets/extra.hpp"
#include "core/PhysicConfig.hpp"
#include "core/tools/Debug.hpp"
#include "core/tools/Error.hpp"
#include "imgui.h"
#include <utility>
using namespace phys::app;

Simulator::Simulator(AppContext &context) : Slide(context)
{
}

void Simulator::tickContent()
{
    if (!this->physic_sim.isRunningSim())
    {
        ImGui::Begin("Preview", nullptr);
        this->review_panel.update(*this->universe);
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

    if (ImGui::BeginTable("SimConfigTable", 4, ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Div", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Mul", ImGuiTableColumnFlags_WidthFixed);

        ImGui::BeginDisabled(this->physic_sim.isRunningSim());

        // Step Method Row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Step Method");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        static const std::vector<std::pair<phys::StepType, const char *>> methods = {
            {phys::StepType::ImplicitEuler, "Implicit Euler"},
            {phys::StepType::Verlet, "Verlet"},
            {phys::StepType::RK4, "RK4"}};
        EnumCombo("##method", this->universe->physicConfig.step_config.step_type, methods);

        float btn_width = ImGui::GetFrameHeight() * 1.8f;

        // Delta Time Row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Delta Time");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputDouble("##deltatime", &this->universe->physicConfig.step_config.delta_time, 0, 0, "%.4e s");
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("/1.5##deltatime", ImVec2(btn_width, 0)))
            this->universe->physicConfig.step_config.delta_time /= 1.5;
        ImGui::TableSetColumnIndex(3);
        if (ImGui::Button("x1.5##deltatime", ImVec2(btn_width, 0)))
            this->universe->physicConfig.step_config.delta_time *= 1.5;

        // Speed Row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Speed");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputDouble("##speed", &this->universe->physicConfig.step_config.speed, 0.0, 0.0, "%.4e x");
        ImGui::TableSetColumnIndex(2);
        if (ImGui::Button("/1.5##speed", ImVec2(btn_width, 0)))
            this->universe->physicConfig.step_config.speed /= 1.5;
        ImGui::TableSetColumnIndex(3);
        if (ImGui::Button("x1.5##speed", ImVec2(btn_width, 0)))
            this->universe->physicConfig.step_config.speed *= 1.5;

        ImGui::EndDisabled();

        if (this->physic_sim.isRunningSim())
        {
            // Sim Speed Row
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Sim Speed");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputDouble("##sim_speed", &this->sim_speed, 0.0, 0.0, "%.4e x"))
                this->physic_sim.speed = this->sim_speed;
            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button("/1.5##sim_speed", ImVec2(btn_width, 0)))
            {
                this->sim_speed /= 1.5;
                this->physic_sim.speed = this->sim_speed;
            }
            ImGui::TableSetColumnIndex(3);
            if (ImGui::Button("x1.5##sim_speed", ImVec2(btn_width, 0)))
            {
                this->sim_speed *= 1.5;
                this->physic_sim.speed = this->sim_speed;
            }
        }
        ImGui::EndTable();
    }

    ImGui::Separator();

    // Simulator Buttons
    if (!this->physic_sim.isRunningSim() && ImGui::Button("Start"))
    {
        this->universe_sim = std::make_shared<Universe>(this->universe->copy());
        auto res = this->physic_sim.startSim(this->universe_sim->env, this->universe_sim->physicConfig);
        if (!res)
        {
            phys::showMessage(res.error().c_str());
        }
    }
    if (this->physic_sim.isRunningSim() && ImGui::Button("Stop"))
    {
        this->physic_sim.stopSim();
    }

    if (this->physic_sim.isRunningSim())
    {
        ImGui::SameLine();
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
