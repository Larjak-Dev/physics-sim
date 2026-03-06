
#include "Player.hpp"
#include "../../tools/Language.hpp"
#include "../widgets/extra.hpp"
#include "imgui.h"
#include "tools/Debug.hpp"
#include "universe/Camera.hpp"
#include "universe/PhysicConfig.hpp"
#include <memory>
#include <ranges>
using namespace phys::app;

Player::Player()
{
}

void Player::multipleScenes()
{

    ImGuiID dock_player = ImGui::GetID("Player_Dock");
    ImGui::DockSpace(dock_player);

    int player_amount = 0;
    for (auto [i, b] : std::views::enumerate(this->selected_recordings))
    {
        if (b == false)
        {
            continue;
        }

        assert(recordings[i]);
        assert(recordings[i]->universe);
        assert(recordings[i]->universe->camera);
        assert(recordings[i]->universe->env);

        const auto &recording = recordings[i];
        const auto &universe = recording->universe;
        const auto &physics_config = universe->physicConfig;
        const auto &frames = recording->getFrames();
        const float frame_amount = frames.size();

        // Player window ID
        player_amount += 1;
        auto player_ID = std::format("Recording {} ({}, dt: {})##Player {}", i,
                                     phys::getStepMetodStr(physics_config.step_config.step_type),
                                     physics_config.step_config.delta_time, player_amount);
        // Player window
        ImGui::Begin(player_ID.c_str(), nullptr);

        // Initialize Scene widget if needed
        if (player_amount >= static_cast<int>(this->scene_widgets.size()))
            this->scene_widgets.resize(player_amount);
        auto &scene_widget = this->scene_widgets[player_amount - 1];

        // Setup Scene widget
        scene_widget.universe->camera = this->scenes_camera;
        int frame_index = std::floor((frame_amount - 1) * this->timeline_select);
        scene_widget.universe->env->setEnvironment_safe(frames[frame_index]);
        scene_widget.universe->env->getProperties_ref() = recording->universe->env->getProperties_ref();

        // Render Scene widget
        scene_widget.update();

        ImGui::End();
    }
}

void Player::almagationScene()
{
    int selected_amount = 0;
    for (auto b : this->selected_recordings)
    {
        if (b == true)
        {
            selected_amount += 1;
        }
    }

    this->scene_widget_alm.resize_ColorSpectrum(selected_amount);
    this->scene_widget_alm.universes[0]->camera = this->scenes_camera;

    int index = 0;
    for (auto [i, b] : std::views::enumerate(this->selected_recordings))
    {
        if (b == false)
        {
            continue;
        }

        assert(recordings[i]);
        assert(recordings[i]->universe);
        assert(recordings[i]->universe->camera);
        assert(recordings[i]->universe->env);

        const auto &recording = recordings[i];
        const auto &universe = recording->universe;
        const auto &frames = recording->getFrames();
        const float frame_amount = frames.size();

        int frame_index = std::floor((frame_amount - 1) * this->timeline_select);
        scene_widget_alm.universes[index]->env->setEnvironment_safe(frames[frame_index]);
        scene_widget_alm.universes[index]->env->getProperties_ref() = universe->env->getProperties_ref();

        index++;
    }

    scene_widget_alm.update();
}

void Player::tickContent()
{
    bool hasSelectedAnything = false;
    for (auto b : this->selected_recordings)
    {
        if (b == true)
        {
            hasSelectedAnything = true;
            break;
        }
    }

    // Players
    if (hasSelectedAnything == true)
    {
        ImGui::Begin("Player");
        almagationScene();
        ImGui::End();
    }

    if (hasSelectedAnything == false)
    {
        // Preview
        ImGui::Begin("Preview", nullptr);
        this->review_panel.update(*this->universe);
        ImGui::End();
    }
}

void Player::tickRightBar()
{

    // Config
    ImGui::Begin("Control Panel");

    ImGui::BeginDisabled(this->simulator.isRunningPreview());

    static const std::vector<std::pair<phys::StepType, const char *>> methods = {
        {phys::StepType::ImplicitEuler, "Implicit Euler"},
        {phys::StepType::Verlet, "Verlet"},
        {phys::StepType::RK4, "RK4"}};
    EnumCombo("Step Method", this->universe->physicConfig.step_config.step_type, methods);

    ImGui::InputDouble("Delta Time", &this->universe->physicConfig.step_config.delta_time);
    ImGui::InputDouble("Total Time", &this->universe->physicConfig.step_config.total_time);

    ImGui::EndDisabled();

    ////Buttons for simulator
    if (!this->simulator.isRunningPreview() && ImGui::Button("Start"))
    {
        // Create recording
        this->recordings.emplace_back(std::make_shared<Recording>());
        auto &recording = this->recordings.back();

        this->scenes_camera = std::make_shared<phys::Camera>(*universe->camera);

        // Start simulating
        this->simulator.startPreview(universe->copy(), recording);
    }
    if (this->simulator.isRunningPreview() && ImGui::Button("Stop"))
    {
        this->simulator.stopPreview();
    }

    if (this->simulator.isRunningPreview())
    {
        if (!this->simulator.isPausedPreview() && ImGui::Button("Pause"))
        {
            this->simulator.pausePreview();
        }
        if (this->simulator.isPausedPreview() && ImGui::Button("Resume"))
        {
            this->simulator.resumePreview();
        }
    }

    ////Status for simulator
    std::string status = "nothing";
    uint32_t completion = 0;

    if (recordings.size() > 0)
    {
        status = recordings.back()->getStatusStr();
        completion = recordings.back()->getCompletion();
    }

    ImGui::Text("%s", status.c_str());
    ImGui::Text("completion: %d", completion);
    // uint32_t amount_frames = recording->getFrames().size();
    //  ImGui::Text("Frame amount: %d", amount_frames);

    ////Recordings resulted from simulator.
    ////Select recordings to view
    ImGui::BeginChild("Recordings", ImVec2(0, 100));
    this->selected_recordings.resize(this->recordings.size());
    for (const auto &&[i, recording] : std::views::enumerate(this->recordings))
    {
        const auto physics_config = recording->universe->physicConfig;
        std::string check_str =
            std::format("Recording {} ({},{})", i, getStepMetodStr(physics_config.step_config.step_type),
                        physics_config.step_config.delta_time);
        ImGui::Checkbox(check_str.c_str(), reinterpret_cast<bool *>(&this->selected_recordings[i]));
    }
    ImGui::EndChild();
    if (ImGui::Button("Clear Recordings"))
    {
        this->recordings.clear();
        this->selected_recordings.clear();
    }

    ////Timeline of current selected recordings
    ImGui::SliderFloat("Timeline", &this->timeline_select, 0.0f, 1.0);

    ImGui::End();
}
