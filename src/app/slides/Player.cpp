
#include "app/slides/Player.hpp"
#include "app/widgets/extra.hpp"
#include "core/tools/Debug.hpp"
#include "core/tools/Language.hpp"
#include "core/universe/Camera.hpp"
#include "core/universe/Environment.hpp"
#include "core/universe/PhysicConfig.hpp"
#include "physics/Kinematics.hpp"
#include "physics/PhysicFunctions.hpp"
#include <OpenXLSX.hpp>
#include <algorithm>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <memory>
#include <ranges>

using namespace phys::app;

Player::Player(AppContext &context) : Slide(context)
{
}

void Player::multipleScenes()
{

    int player_amount = 0;
    for (auto &&[recording, b] : this->recordings)
    {
        if (b == false)
        {
            continue;
        }

        assert(recording);
        assert(recording->universe);
        assert(recording->universe->camera);
        assert(recording->universe->env);

        const auto &universe = recording->universe;
        const auto &physics_config = universe->physicConfig;
        const auto &frames = recording->getFrames();
        const float frame_amount = frames.size();

        // Player window ID
        player_amount += 1;
        auto player_ID = std::format("Recording {} ({}, dt: {})##Player {}", player_amount,
                                     phys::getStepMetodStr(physics_config.step_config.step_type),
                                     physics_config.step_config.delta_time, player_amount);
        // Player window
        ImGui::Begin(player_ID.c_str(), nullptr);

        // Initialize Scene widget if needed
        while (player_amount > static_cast<int>(this->scene_widgets.size()))
            this->scene_widgets.push_back(std::make_unique<UniverseWidget>(this->context));
        auto &scene_widget = *this->scene_widgets[player_amount - 1];

        // Setup Scene widget
        scene_widget.universe->camera = this->scenes_camera;
        int frame_index = std::round((frame_amount - 1) * this->timeline_passed_ratio);
        scene_widget.universe->env->setEnvironment_safe(frames[frame_index]);
        scene_widget.universe->env->getProperties_ref() = recording->universe->env->getProperties_ref();

        // Render Scene widget
        scene_widget.update(true);

        ImGui::End();
    }
}

void Player::almagationScene()
{

    int selected_amount = 0;
    const auto &item_first = std::ranges::find_if(this->recordings, [](const auto &item) { return item.second; });
    for (auto &&[recording, b] : this->recordings)
    {
        if (b)
            selected_amount++;
    }

    const auto &recording_first = item_first->first;

    const auto &frames_first = recording_first->getFrames();
    const float frame_amount_first = frames_first.size();
    int frame_index_first = std::round((frame_amount_first - 1) * this->timeline_passed_ratio);

    const auto env_first = recording_first->getFrames()[frame_index_first];
    this->scene_widget_alm.universe->camera = this->scenes_camera;
    this->scene_widget_alm.universe->env->setEnvironment_safe(env_first);
    this->scene_widget_alm.universe->env->getProperties_ref() = universe->env->getProperties_ref();

    bool has_kinematic = static_cast<Environment>(*recording_first->universe->env).config.is_calculated;

    this->scene_widget_alm.resize(has_kinematic + selected_amount);
    this->scene_widget_alm.universes[0]->camera = this->scenes_camera;

    // Kinematic rendering
    if (has_kinematic)
    {
        this->scene_widget_alm.properties[0] = {0.8f, Color(0.5, 0.5, 0.5)};
        const auto time = recording_first->total_time * static_cast<double>(this->timeline_passed_ratio);
        const auto env = static_cast<Environment>(*recording_first->universe->env);
        const auto body_kinematic = phys::calcBody(env.config, time);

        EnvironmentBase env_kinematic;
        env_kinematic.bodies.emplace_back(body_kinematic);
        this->scene_widget_alm.universes[0]->env->setEnvironment_safe(env_kinematic);
        this->scene_widget_alm.universes[0]->env->getProperties_ref() = universe->env->getProperties_ref();
    }

    // Selected Environments rendering
    int alm_index = has_kinematic;
    for (auto &&[i, item] : std::views::enumerate(this->recordings))
    {
        auto &&[recording, b] = item;
        if (b == false)
        {
            continue;
        }

        assert(recording);
        assert(recording->universe);
        assert(recording->universe->camera);
        assert(recording->universe->env);

        const auto &universe = recording->universe;
        const auto &frames = recording->getFrames();
        const float frame_amount = frames.size();

        // Stable color based on original list index
        float hue = std::fmod(static_cast<float>(i) * 0.618033988749895f, 1.0f);
        this->scene_widget_alm.properties[alm_index] = {1.0f, hueToRGB(hue)};

        int frame_index = std::round((frame_amount - 1) * this->timeline_passed_ratio);
        this->scene_widget_alm.universes[alm_index]->env->setEnvironment_safe(frames[frame_index]);
        this->scene_widget_alm.universes[alm_index]->env->getProperties_ref() = universe->env->getProperties_ref();

        alm_index++;
    }

    this->scene_widget_alm.update();
}

void Player::tickContent()
{
    bool has_selected_anything = false;
    for (auto [recording, b] : this->recordings)
    {
        if (b == true)
        {
            has_selected_anything = true;
            break;
        }
    }

    // Players
    if (has_selected_anything == true)
    {
        using namespace ImGui;
        ImGui::Begin("Player");
        ImGuiID dock_player = ImGui::GetID("Player_Dock");
        ImGui::DockSpace(dock_player, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        this->buildDock(dock_player);
        ImGui::End();

        ImGui::Begin("Scene (Player)", nullptr, ImGuiWindowFlags_NoMove);
        almagationScene();
        ImGui::End();

        ImGui::Begin("Timeline", nullptr, ImGuiWindowFlags_NoMove);

        ////Timeline of current selected recordings
        const auto first_item = std::ranges::find_if(this->recordings, [](const auto &item) { return item.second; });
        if (first_item != this->recordings.end())
        {
            const auto &recording = first_item->first;
            const float total_time = recording->total_time;
            const size_t total_frames = recording->getFrames().size();
            
            // Unified frame calculation from ratio
            auto get_frame_from_ratio = [&](float ratio) {
                return std::clamp(static_cast<int>(std::round(ratio * (total_frames - 1))), 0, static_cast<int>(total_frames - 1));
            };

            int current_frame_i = get_frame_from_ratio(this->timeline_passed_ratio);
            const float current_time = this->timeline_passed_ratio * total_time;

            // Playback logic
            if (this->is_playing)
            {
                float dt = ImGui::GetIO().DeltaTime * this->playback_speed;
                float new_time = current_time + dt;
                if (new_time >= total_time)
                {
                    new_time = 0.0f; // Loop
                }
                this->timeline_passed_ratio = new_time / total_time;
            }

            // UI Layout
            float available_width = ImGui::GetContentRegionAvail().x;
            
            // --- COOLER SLIDER START ---
            float grab_sz = 20.0f;
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, grab_sz);
            
            ImGui::SetNextItemWidth(available_width);
            
            // Use SliderInt for the frame index to ensure 100% click accuracy
            if (ImGui::SliderInt("##TimelineSlider", &current_frame_i, 0, total_frames - 1, ""))
            {
                this->timeline_passed_ratio = static_cast<float>(current_frame_i) / (total_frames - 1);
                this->is_playing = false;
            }

            ImVec2 slider_min = ImGui::GetItemRectMin();
            ImVec2 slider_max = ImGui::GetItemRectMax();

            // Accurate ratio calculation matching ImGui internal SliderBehaviorT for SliderInt
            float slider_sz = (slider_max.x - slider_min.x) - 4.0f; // 2.0f grab_padding on each side
            float actual_grab_sz = std::max(slider_sz / static_cast<float>(total_frames), grab_sz);
            float usable_w = slider_sz - actual_grab_sz;
            float usable_x_min = slider_min.x + 2.0f + actual_grab_sz * 0.5f;

            auto get_ratio_from_x = [&](float x) {
                if (usable_w <= 0.0f) return 0.0f;
                return std::clamp((x - usable_x_min) / usable_w, 0.0f, 1.0f);
            };

            // Hover Tooltip: Preview time at mouse position
            if (ImGui::IsItemHovered())
            {
                float hover_ratio = get_ratio_from_x(ImGui::GetMousePos().x);
                int hover_frame = get_frame_from_ratio(hover_ratio);
                float snapped_hover_ratio = static_cast<float>(hover_frame) / (total_frames - 1);
                float hover_time = snapped_hover_ratio * total_time;
                
                ImGui::BeginTooltip();
                ImGui::Text("Jump to: %.3f s (Frame %d)", hover_time, hover_frame + 1);
                ImGui::EndTooltip();
            }

            // Custom "Played" progress bar overlay using theme colors
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            // Calculate progress bar width to end exactly at the center of the grabber
            float fill_x_end = usable_x_min + (this->timeline_passed_ratio * usable_w);
            
            if (fill_x_end > slider_min.x + 4.0f) {
                ImU32 fill_col = ImGui::GetColorU32(ImGuiCol_HeaderActive, 0.5f);
                draw_list->AddRectFilled(
                    slider_min, 
                    ImVec2(fill_x_end, slider_max.y), 
                    fill_col, 12.0f
                );
            }

            ImGui::PopStyleVar(3);
            // --- COOLER SLIDER END ---

            // Second row: Controls and Info
            float btn_size = ImGui::GetFrameHeight();
            
            // Buttons group
            if (ImGui::Button(this->is_playing ? "Pause" : "Play", ImVec2(btn_size * 2.5f, 0)))
            {
                this->is_playing = !this->is_playing;
            }
            ImGui::SameLine();
            if (ImGui::Button("|<", ImVec2(btn_size, 0))) { setFrameIndex(0); this->is_playing = false; }
            ImGui::SameLine();
            if (ImGui::Button("<", ImVec2(btn_size, 0))) { stepTimeline(-1); this->is_playing = false; }
            ImGui::SameLine();
            if (ImGui::Button(">", ImVec2(btn_size, 0))) { stepTimeline(1); this->is_playing = false; }
            ImGui::SameLine();
            if (ImGui::Button(">|", ImVec2(btn_size, 0))) { setFrameIndex(total_frames - 1); this->is_playing = false; }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(btn_size * 3.0f);
            ImGui::DragFloat("##Speed", &this->playback_speed, 0.05f, 0.1f, 10.0f, "x%.1f");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Playback Speed");

            // Time/Frame info on the right
            std::string info_str = std::format("{:.2f} / {:.2f}s | Frame {} / {}", 
                                               current_time, total_time, current_frame_i + 1, total_frames);
            float text_width = ImGui::CalcTextSize(info_str.c_str()).x;
            ImGui::SameLine(available_width - text_width);
            ImGui::TextUnformatted(info_str.c_str());

            ImGui::End();
        }
    }

    if (has_selected_anything == false)
    {
        // Preview
        ImGui::Begin("Preview", nullptr);
        this->review_panel.update(*this->universe);
        ImGui::End();
    }
}

void Player::buildDock(ImGuiID dock_player)
{
    static bool first = true;

    if (!first)
        return;
    first = false;

    using namespace ImGui;
    DockBuilderRemoveNode(dock_player);
    DockBuilderAddNode(dock_player, ImGuiDockNodeFlags_DockSpace);
    DockBuilderSetNodeSize(dock_player, ImGui::GetWindowSize());

    ImGuiID dock_main = dock_player;
    ImGuiID dock_bot = DockBuilderSplitNode(dock_player, ImGuiDir_Down, 0.3f, nullptr, &dock_main);

    DockBuilderDockWindow("Scene (Player)", dock_main);
    DockBuilderDockWindow("Timeline", dock_bot);
}

void Player::stepTimeline(int i)
{
    const auto &first_item = std::ranges::find_if(this->recordings, [](const auto &item) { return item.second; });
    if (first_item == this->recordings.end())
        return;
    const auto &recording = first_item->first;

    size_t frames_size = recording->getFrames().size();
    if (frames_size <= 1)
        return;

    int current_frame_index = static_cast<int>(std::round(this->timeline_passed_ratio * (frames_size - 1)));
    int index_new = current_frame_index + i;

    if (index_new < 0)
        index_new = 0;
    if (index_new >= static_cast<int>(frames_size))
        index_new = static_cast<int>(frames_size) - 1;

    setFrameIndex(static_cast<unsigned int>(index_new));
}

std::string last_save = "";

void Player::saveAsExcel()
{
    namespace fs = std::filesystem;
    using namespace OpenXLSX;

    auto force_type = phys::getForceStr(this->universe->physicConfig.force_config.force_type);

    // Ensure directory exists
    fs::create_directories("Excel_Output");

    std::string doc_path = std::format("Excel_Output/{}.xlsx", force_type);
    if (fs::exists(doc_path))
    {
        int count = 1;
        while (fs::exists(std::format("Excel_Output/{}_{}.xlsx", force_type, count)))
        {
            count++;
        }
        doc_path = std::format("Excel_Output/{}_{}.xlsx", force_type, count);
    }
    last_save = doc_path;

    XLDocument doc;
    doc.create(doc_path);
    auto wks = doc.workbook().worksheet("Sheet1");

    int x_index = 1;
    for (auto &&[recording, b] : this->recordings)
    {
        if (b)
        {
            auto environment = static_cast<Environment>(*recording->universe->env);
            auto universe_config = environment.config;
            bool is_kinematic = universe_config.is_calculated;

            // Row 1
            wks.cell(1, x_index).value() =
                phys::getStepMetodStr(recording->universe->physicConfig.step_config.step_type);
            wks.cell(1, x_index + 1).value() =
                std::format("Delta Time: {}", recording->universe->physicConfig.step_config.delta_time);

            // DATA Labels
            int y_label = 3;
            wks.cell(y_label, x_index).value() = "Time:";
            wks.cell(y_label, x_index + 1).value() = "Index:";
            
            if (is_kinematic) {
                wks.cell(y_label, x_index + 2).value() = "X (Kinematic):";
                wks.cell(y_label, x_index + 3).value() = "Y (Kinematic):";
                wks.cell(y_label, x_index + 4).value() = "Z (Kinematic):";
                wks.cell(y_label, x_index + 5).value() = "X:";
                wks.cell(y_label, x_index + 6).value() = "Y:";
                wks.cell(y_label, x_index + 7).value() = "Z:";
                wks.cell(y_label, x_index + 8).value() = "Delta Magnitude:";
                wks.cell(y_label, x_index + 9).value() = "Velocity (Kinematic):";
                wks.cell(y_label, x_index + 10).value() = "Velocity:";
                wks.cell(y_label, x_index + 11).value() = "Delta Velocity:";
                wks.cell(y_label, x_index + 12).value() = "Energy Total (Kinematic):";
                wks.cell(y_label, x_index + 13).value() = "Energy Total:";
                wks.cell(y_label, x_index + 14).value() = "Delta Energy:";
                wks.cell(y_label, x_index + 15).value() = "Delta Energy (Abs):";
                wks.cell(y_label, x_index + 16).value() = "####";
            } else {
                wks.cell(y_label, x_index + 2).value() = "X:";
                wks.cell(y_label, x_index + 3).value() = "Y:";
                wks.cell(y_label, x_index + 4).value() = "Z:";
                wks.cell(y_label, x_index + 5).value() = "Velocity:";
                wks.cell(y_label, x_index + 6).value() = "Energy Total:";
                wks.cell(y_label, x_index + 7).value() = "####";
            }

            // DATA Values

            double magnitude_delta_sum = 0.0f;
            double velocity_delta_sum = 0.0f;
            double energy_delta_sum = 0.0f;

            int y_values = y_label + 1;
            const auto physic_functions = phys::PhysicFunctions(recording->universe->physicConfig);
            const auto &frames = recording->getFrames();
            for (auto &&[i, env] : std::views::enumerate(frames))
            {
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
                const auto x = body.pos.x;
                const auto y = body.pos.y;
                const auto z = body.pos.z;
                const auto velocity = glm::length(body.vel);
                
                auto energy_k = 0.5 * body.mass * std::pow(velocity, 2);
                auto energy_p = 0.0;

                switch (universe_config.force_config.force_type)
                {
                case phys::ForceType::FreeFall:
                {
                    const auto g = universe_config.force_config.freefall_config.g;
                    energy_p = body.mass * g * body.pos.y;
                }
                break;
                case phys::ForceType::Newtonian:
                {
                    const auto G = universe_config.force_config.newtonian_config.G;
                    const auto M = universe_config.mass_2_newtonian;
                    const auto r = glm::length(body.pos);
                    energy_p = -G * M * body.mass / r;
                }
                break;
                default:
                    break;
                }

                auto energy_t = energy_p + energy_k;

                if (is_kinematic) {
                    const auto kinematic_body = phys::calcBody(universe_config, env.passed_time);
                    const auto x_k = kinematic_body.pos.x;
                    const auto y_k = kinematic_body.pos.y;
                    const auto z_k = kinematic_body.pos.z;
                    const auto magnitude_delta = glm::length(body.pos - kinematic_body.pos);
                    const auto velocity_k = glm::length(kinematic_body.vel);
                    const auto velocity_delta = velocity - velocity_k;
                    
                    auto energy_k_k = 0.5 * kinematic_body.mass * std::pow(velocity_k, 2);
                    auto energy_p_k = 0.0;
                    if (universe_config.force_config.force_type == ForceType::FreeFall) {
                        energy_p_k = kinematic_body.mass * universe_config.force_config.freefall_config.g * kinematic_body.pos.y;
                    } else if (universe_config.force_config.force_type == ForceType::Newtonian) {
                        energy_p_k = -universe_config.force_config.newtonian_config.G * universe_config.mass_2_newtonian * kinematic_body.mass / glm::length(kinematic_body.pos);
                    }
                    
                    auto energy_t_k = energy_p_k + energy_k_k;
                    auto energy_t_delta = energy_t - energy_t_k;
                    auto energy_t_delta_abs = glm::abs(energy_t_delta);

                    wks.cell(y_values + i, x_index).value() = time;
                    wks.cell(y_values + i, x_index + 1).value() = index;
                    wks.cell(y_values + i, x_index + 2).value() = x_k;
                    wks.cell(y_values + i, x_index + 3).value() = y_k;
                    wks.cell(y_values + i, x_index + 4).value() = z_k;
                    wks.cell(y_values + i, x_index + 5).value() = x;
                    wks.cell(y_values + i, x_index + 6).value() = y;
                    wks.cell(y_values + i, x_index + 7).value() = z;
                    wks.cell(y_values + i, x_index + 8).value() = magnitude_delta;
                    wks.cell(y_values + i, x_index + 9).value() = velocity_k;
                    wks.cell(y_values + i, x_index + 10).value() = velocity;
                    wks.cell(y_values + i, x_index + 11).value() = velocity_delta;
                    wks.cell(y_values + i, x_index + 12).value() = energy_t_k;
                    wks.cell(y_values + i, x_index + 13).value() = energy_t;
                    wks.cell(y_values + i, x_index + 14).value() = energy_t_delta;
                    wks.cell(y_values + i, x_index + 15).value() = energy_t_delta_abs;
                    wks.cell(y_values + i, x_index + 16).value() = "####";

                    magnitude_delta_sum += magnitude_delta;
                    velocity_delta_sum += velocity_delta;
                    energy_delta_sum += energy_t_delta_abs;
                } else {
                    wks.cell(y_values + i, x_index).value() = time;
                    wks.cell(y_values + i, x_index + 1).value() = index;
                    wks.cell(y_values + i, x_index + 2).value() = x;
                    wks.cell(y_values + i, x_index + 3).value() = y;
                    wks.cell(y_values + i, x_index + 4).value() = z;
                    wks.cell(y_values + i, x_index + 5).value() = velocity;
                    wks.cell(y_values + i, x_index + 6).value() = energy_t;
                    wks.cell(y_values + i, x_index + 7).value() = "####";
                }
            }

            if (is_kinematic) {
                const auto magnitude_delta_average = magnitude_delta_sum / recording->getFrames().size();
                const auto velocity_delta_average = velocity_delta_sum / recording->getFrames().size();
                const auto energy_delta_average = energy_delta_sum / recording->getFrames().size();

                // Averages
                wks.cell(1, x_index + 7).value() = "Magnitude Delta Average: ";
                wks.cell(1, x_index + 8).value() = magnitude_delta_average;
                wks.cell(1, x_index + 10).value() = "Velocity Delta Average";
                wks.cell(1, x_index + 11).value() = velocity_delta_average;
                wks.cell(1, x_index + 14).value() = "Energy Delta Average";
                wks.cell(1, x_index + 15).value() = energy_delta_average;

                x_index += 17;
            } else {
                x_index += 8;
            }
        }
    }

    doc.save();
}

void Player::tickRightBar()
{
    // Config
    ImGui::Begin("Control Panel");

    if (ImGui::BeginTable("PlayerConfigTable", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.6f);

        ImGui::BeginDisabled(this->simulator.isRunningPreview());

        // Step Method Row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Step Method");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        static const std::vector<std::pair<phys::StepType, const char *>> methods = {
            {phys::StepType::ImplicitEuler, "Implicit Euler"}, {phys::StepType::Verlet, "Verlet"}, {phys::StepType::RK4, "RK4"}};
        EnumCombo("##method", this->universe->physicConfig.step_config.step_type, methods);

        // Delta Time Row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Delta Time");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputDouble("##deltatime", &this->universe->physicConfig.step_config.delta_time, 0, 0, "%.4e s");

        ImGui::EndDisabled();

        // Status for simulator
        std::string status = "nothing";
        uint32_t completion = 0;

        if (recordings.size() > 0)
        {
            auto &recording_back = recordings.back().first;
            status = recording_back->getStatusStr();
            completion = recording_back->getCompletion();
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Status");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", status.c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Completion");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d%%", completion);

        ImGui::EndTable();
    }

    ImGui::Separator();

    ////Buttons for simulator
    if (!this->simulator.isRunningPreview() && ImGui::Button("Start", ImVec2(-FLT_MIN, 0)))
    {
        // Create recording
        this->recordings.emplace_back(std::make_shared<Recording>(), false);
        auto &recording = this->recordings.back().first;

        this->scenes_camera = std::make_shared<phys::Camera>(*universe->camera);

        // Start simulating
        this->simulator.startPreview(universe->copy(), recording);
    }
    if (this->simulator.isRunningPreview())
    {
        if (ImGui::Button("Stop", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
        {
            this->simulator.stopPreview();
        }
        ImGui::SameLine();
        if (!this->simulator.isPausedPreview() && ImGui::Button("Pause", ImVec2(-FLT_MIN, 0)))
        {
            this->simulator.pausePreview();
        }
        if (this->simulator.isPausedPreview() && ImGui::Button("Resume", ImVec2(-FLT_MIN, 0)))
        {
            this->simulator.resumePreview();
        }
    }

    ImGui::SeparatorText("Recordings");

    ////Recordings resulted from simulator.
    ////Select recordings to view
    int selected_count = 0;
    for (auto &&[r, b] : this->recordings)
        if (b)
            selected_count++;

    ImGui::BeginChild("RecordingsList", ImVec2(0, 150), ImGuiChildFlags_Borders);
    for (auto &&[i, item] : std::views::enumerate(this->recordings))
    {
        auto &&[recording, b] = item;
        if (recording->getStatus() >= 1)
        {
            ImGui::BeginDisabled(recording->getStatus() < 3);

            bool pushed_color = false;
            if (b)
            {
                float hue = std::fmod(static_cast<float>(i) * 0.618033988749895f, 1.0f);
                Color c = hueToRGB(hue);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(c.r, c.g, c.b, 1.0f));
                pushed_color = true;
            }

            const auto physics_config = recording->universe->physicConfig;
            std::string check_str =
                std::format("Recording {} ({}, dt: {:.1e})", i + 1, getStepMetodStr(physics_config.step_config.step_type),
                            physics_config.step_config.delta_time);
            ImGui::Checkbox(check_str.c_str(), reinterpret_cast<bool *>(&b));

            if (pushed_color)
                ImGui::PopStyleColor();

            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();

    if (ImGui::Button("Clear Recordings", ImVec2(-FLT_MIN, 0)))
    {
        this->recordings.clear();
    }

    ImGui::Separator();

    // Save Excel
    if (ImGui::Button("Save Excel", ImVec2(-FLT_MIN, 0)))
    {
        this->saveAsExcel();
    }
    if (!last_save.empty())
    {
        ImGui::TextWrapped("Last saved: %s", last_save.c_str());
    }

    ImGui::End();
}

// Sets the timeline float to an specific frame index of the first selected recording.
void Player::setFrameIndex(unsigned int i)
{

    const auto &first_item = std::ranges::find_if(this->recordings, [](const auto &item) { return item.second; });
    if (first_item == this->recordings.end())
        return;
    const auto &recording = first_item->first;

    size_t frames_size = recording->getFrames().size();
    if (frames_size <= 1)
    {
        this->timeline_passed_ratio = 0.0f;
        return;
    }

    if (i >= frames_size)
        i = frames_size - 1;

    this->timeline_passed_ratio = static_cast<float>(i) / (frames_size - 1);
}
