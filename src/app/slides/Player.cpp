
#include "Player.hpp"
#include "../../tools/Language.hpp"
#include "../widgets/extra.hpp"
#include "imgui.h"
#include "physics/Kinematics.hpp"
#include "physics/PhysicFunctions.hpp"
#include "tools/Debug.hpp"
#include "universe/Camera.hpp"
#include "universe/Environment.hpp"
#include "universe/PhysicConfig.hpp"
#include <OpenXLSX.hpp>
#include <filesystem>
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
    int first_selection = -1;
    for (auto [i, b] : std::views::enumerate(this->selected_recordings))
    {
        if (b == true)
        {
            selected_amount += 1;
            if (first_selection == -1)
                first_selection = i;
        }
    }
    const auto &recording_first = recordings[first_selection];
    int has_kinematic = recording_first->getKinematicFrames().size() > 0;

    this->scene_widget_alm.resize_ColorSpectrum(has_kinematic + selected_amount);
    this->scene_widget_alm.universes[0]->camera = this->scenes_camera;

    // Kinematic rendering
    if (has_kinematic)
    {
        this->scene_widget_alm.properties[0].first = 0.8;
        this->scene_widget_alm.properties[0].second = Color(0.5, 0.5, 0.5);
        const auto time = recording_first->total_time * static_cast<double>(this->timeline_select);
        const auto env = static_cast<Environment>(*recording_first->universe->env);
        const auto body_kinematic = phys::calcBody(env.config, time);

        EnvironmentBase env_kinematic;
        env_kinematic.bodies.emplace_back(body_kinematic);
        this->scene_widget_alm.universes[0]->env->setEnvironment_safe(env_kinematic);
        this->scene_widget_alm.universes[0]->env->getProperties_ref() = universe->env->getProperties_ref();
    }

    // Selected Environments rendering
    int index = has_kinematic;
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
        this->scene_widget_alm.universes[index]->env->setEnvironment_safe(frames[frame_index]);
        this->scene_widget_alm.universes[index]->env->getProperties_ref() = universe->env->getProperties_ref();

        index++;
    }

    this->scene_widget_alm.update();
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

std::string last_save = "";

void Player::saveAsExcel()
{
    namespace fs = std::filesystem;
    using namespace OpenXLSX;

    auto forceType = phys::getForceStr(this->universe->physicConfig.force_config.force_type);

    // Ensure directory exists
    fs::create_directories("Excel_Output");

    std::string doc_path = std::format("Excel_Output/{}.xlsx", forceType);
    if (fs::exists(doc_path))
    {
        int count = 1;
        while (fs::exists(std::format("Excel_Output/{}_{}.xlsx", forceType, count)))
        {
            count++;
        }
        doc_path = std::format("Excel_Output/{}_{}.xlsx", forceType, count);
    }
    last_save = doc_path;

    XLDocument doc;
    doc.create(doc_path);
    auto wks = doc.workbook().worksheet("Sheet1");

    int index_X = 1;
    for (auto &&[i, b] : std::views::enumerate(this->selected_recordings))
    {
        if (b)
        {
            auto &recording = this->recordings[i];
            auto environment = static_cast<Environment>(*recording->universe->env);
            auto universe_config = environment.config;

            // Rows 1
            wks.cell(1, index_X).value() =
                phys::getStepMetodStr(recording->universe->physicConfig.step_config.step_type);
            wks.cell(1, index_X + 1).value() =
                std::format("Delta Time: {}", recording->universe->physicConfig.step_config.delta_time);

            // Row 2
            int start_y = 3;
            wks.cell(start_y, index_X).value() = "Time:";
            wks.cell(start_y, index_X + 1).value() = "Index:";
            wks.cell(start_y, index_X + 2).value() = "X (Kinematic):";
            wks.cell(start_y, index_X + 3).value() = "Y (Kinematic):";
            wks.cell(start_y, index_X + 4).value() = "Z (Kinematic):";
            wks.cell(start_y, index_X + 5).value() = "X:";
            wks.cell(start_y, index_X + 6).value() = "Y:";
            wks.cell(start_y, index_X + 7).value() = "Z:";
            wks.cell(start_y, index_X + 8).value() = "Delta Magnitude:";
            wks.cell(start_y, index_X + 9).value() = "Velocity (Kinematic):";
            wks.cell(start_y, index_X + 10).value() = "Velocity:";
            wks.cell(start_y, index_X + 11).value() = "Delta Velocity:";
            wks.cell(start_y, index_X + 12).value() = "Energy Total (Kinematic):";
            wks.cell(start_y, index_X + 13).value() = "Energy Total:";
            wks.cell(start_y, index_X + 14).value() = "Delta Energy:";
            wks.cell(start_y, index_X + 15).value() = "Delta Energy (Abs):";
            wks.cell(start_y, index_X + 16).value() = "####";

            double magnitude_delta_sum = 0.0f;
            double velocity_delta_sum = 0.0f;
            double energy_delta_sum = 0.0f;

            int startY = start_y + 1;
            const auto physic_functions = phys::PhysicFunctions(recording->universe->physicConfig);
            const auto &frames = recording->getFrames();
            for (auto &&[i, env] : std::views::enumerate(frames))
            {
                const auto kinematic_body = phys::calcBody(universe_config, env.passed_time);
                auto body = env.bodies[0];
                const double dt = recording->universe->physicConfig.step_config.delta_time;

                if (recording->universe->physicConfig.step_config.step_type == StepType::Verlet)
                {
                    // More accurate Verlet velocity (v = (pos - prev_pos)/dt + 0.5 * a * dt)
                    const auto acceleration = physic_functions.acceleration(body.pos, body, env);
                    body.vel = (body.pos - body.prev_pos) / dt + 0.5 * acceleration * dt;
                }

                const auto time = env.passed_time;
                const auto index = i;
                const auto x_k = kinematic_body.pos.x;
                const auto y_k = kinematic_body.pos.y;
                const auto z_k = kinematic_body.pos.z;
                const auto x = body.pos.x;
                const auto y = body.pos.y;
                const auto z = body.pos.z;
                const auto magnitude_delta = glm::length(body.pos - kinematic_body.pos);
                const auto velocity_k = glm::length(kinematic_body.vel);
                const auto velocity = glm::length(body.vel);
                const auto velocity_delta = velocity - velocity_k;
                auto energy_k_k = 0.5 * kinematic_body.mass * std::pow(velocity_k, 2);
                auto energy_p_k = 0.0;
                auto energy_t_k = 0.0;
                auto energy_k = 0.5 * body.mass * std::pow(velocity, 2);
                auto energy_p = 0.0;
                auto energy_t = 0.0;
                auto energy_t_delta = 0.0;
                auto energy_t_delta_abs = 0.0;

                switch (universe_config.force_config.force_type)
                {
                case phys::ForceType::FreeFall:
                {
                    const auto g = universe_config.force_config.freefall_config.g;
                    energy_p_k = kinematic_body.mass * g * kinematic_body.pos.y;
                    energy_p = body.mass * g * body.pos.y;
                }
                break;
                case phys::ForceType::Newtonian:
                {
                    const auto G = universe_config.force_config.newtonian_config.G;
                    const auto M = universe_config.mass_2_newtonian;

                    const auto r_k = glm::length(kinematic_body.pos);
                    energy_p_k = -G * M * kinematic_body.mass / r_k;

                    const auto r = glm::length(body.pos);
                    energy_p = -G * M * body.mass / r;
                }
                break;
                default:
                    break;
                }

                energy_t_k = energy_p_k + energy_k_k;
                energy_t = energy_p + energy_k;
                energy_t_delta = energy_t - energy_t_k;
                energy_t_delta_abs = glm::abs(energy_t_delta);

                wks.cell(startY + i, index_X).value() = time;
                wks.cell(startY + i, index_X + 1).value() = index;
                wks.cell(startY + i, index_X + 2).value() = x_k;
                wks.cell(startY + i, index_X + 3).value() = y_k;
                wks.cell(startY + i, index_X + 4).value() = z_k;
                wks.cell(startY + i, index_X + 5).value() = x;
                wks.cell(startY + i, index_X + 6).value() = y;
                wks.cell(startY + i, index_X + 7).value() = z;
                wks.cell(startY + i, index_X + 8).value() = magnitude_delta;
                wks.cell(startY + i, index_X + 9).value() = velocity_k;
                wks.cell(startY + i, index_X + 10).value() = velocity;
                wks.cell(startY + i, index_X + 11).value() = velocity_delta;
                wks.cell(startY + i, index_X + 12).value() = energy_t_k;
                wks.cell(startY + i, index_X + 13).value() = energy_t;
                wks.cell(startY + i, index_X + 14).value() = energy_t_delta;
                wks.cell(startY + i, index_X + 15).value() = energy_t_delta_abs;
                wks.cell(startY + i, index_X + 16).value() = "####";

                magnitude_delta_sum += magnitude_delta;
                velocity_delta_sum += velocity_delta;
                energy_delta_sum += energy_t_delta_abs;
            }

            const auto magnitude_delta_average = magnitude_delta_sum / recording->getFrames().size();
            const auto velocity_delta_average = velocity_delta_sum / recording->getFrames().size();
            const auto energy_delta_average = energy_delta_sum / recording->getFrames().size();

            // Averages
            wks.cell(1, index_X + 7).value() = "Magnitude Delta Average: ";
            wks.cell(1, index_X + 8).value() = magnitude_delta_average;
            wks.cell(1, index_X + 10).value() = "Velocity Delta Average";
            wks.cell(1, index_X + 11).value() = velocity_delta_average;
            wks.cell(1, index_X + 14).value() = "Energy Delta Average";
            wks.cell(1, index_X + 15).value() = energy_delta_average;

            index_X += 17;
        }
    }

    doc.save();
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
        if (recording->getStatus() >= 1)
        {
            ImGui::BeginDisabled(recording->getStatus() < 3);
            const auto physics_config = recording->universe->physicConfig;
            std::string check_str =
                std::format("Recording {} ({},{})", i, getStepMetodStr(physics_config.step_config.step_type),
                            physics_config.step_config.delta_time);
            ImGui::Checkbox(check_str.c_str(), reinterpret_cast<bool *>(&this->selected_recordings[i]));
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    if (ImGui::Button("Clear Recordings"))
    {
        this->recordings.clear();
        this->selected_recordings.clear();
    }

    ////Timeline of current selected recordings
    ImGui::SliderFloat("Timeline", &this->timeline_select, 0.0f, 1.0);

    if (ImGui::Button("Save Excel"))
    {
        this->saveAsExcel();
    }
    ImGui::Text(last_save.c_str());

    ImGui::End();
}
