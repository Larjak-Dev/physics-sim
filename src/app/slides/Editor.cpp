
#include "app/slides/Editor.hpp"
#include "app/widgets/extra.hpp"
#include "core/tools/Debug.hpp"
#include "core/tools/Error.hpp"
#include "core/universe/PhysicConfig.hpp"
#include "physics/Kinematics.hpp"
#include "physics/PlanetAPI.hpp"
#include <imgui.h>
#include <utility>
using namespace phys::app;

Editor::Editor(AppContext &context) : Slide(context)
{
    this->universe_type = PresetType::Kinematic;
}

void Editor::tickContent()
{
    ImGui::Begin("Editor", nullptr);
    this->review_panel.update(*this->universe, true);
    ImGui::End();
}

void Editor::tickKinematic(std::shared_ptr<Universe> &universe_main)
{
    if (ImGui::BeginTable("KinematicInputTable", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.6f);

        // Kinematic Environment Row
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Environment");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        static const std::vector<std::pair<phys::ForceType, const char *>> force_types = {
            {phys::ForceType::FreeFall, "Free Fall"}, {phys::ForceType::Newtonian, "Satelite Orbit"}};
        EnumCombo("##kinematic_env", this->kinematic_config.type, force_types);

        switch (this->kinematic_config.type)
        {
        case phys::ForceType::FreeFall:
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Acceleration");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputDouble("##acceleration", &kinematic_config.acceleration, 0, 0, "%.4e m/s^2");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Total Time");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputDouble("##time_fall", &kinematic_config.time_fall, 0, 0, "%.4e s");
        }
        break;
        case phys::ForceType::Newtonian:
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("G");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputDouble("##G", &kinematic_config.G, 0, 0, "%.6e F/kg");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Mass Planet");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputDouble("##mass_planet", &kinematic_config.mass_planet, 0, 0, "%.4e");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Distance");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputDouble("##distance", &kinematic_config.distance, 0, 0, "%.4e m");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Total Time");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputDouble("##time_sat", &kinematic_config.time_satelite, 0, 0, "%.4e s");
        }
        break;
        default:
            break;
        }
        ImGui::EndTable();
    }

    if (ImGui::Button("Configure", ImVec2(-FLT_MIN, 0)))
    {
        if (this->kinematic_config.type == ForceType::Null)
        {
            showMessage("Unvalid kinematic config");
        }
        else
        {
            auto config_uni = phys::createKinematicScenario(this->kinematic_config);
            universe_main = std::make_shared<Universe>(createUniverse(config_uni));
        }
    }
}

void Editor::tickSolarSystem(std::shared_ptr<Universe> &universe_main)
{
    if (ImGui::BeginTable("SolarSystemInputTable", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.6f);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Year");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputInt("##year", &this->planet_api.year_1);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Month");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputInt("##month", &this->planet_api.month_1);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Day");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputInt("##day", &this->planet_api.day_1);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Total Time");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputDouble("##total_days", &this->planet_api.total_days, 0, 0, "%.2f Days");

        ImGui::EndTable();
    }

    if (ImGui::Button("Configure", ImVec2(-FLT_MIN, 0)))
    {
        {
            auto solar_systems = planet_api.fetchSolarSystem();
            universe_main = std::make_shared<Universe>(solar_systems.universe_1);
        }
    }
}

void Editor::tickRightBar(std::shared_ptr<Universe> &universe_main)
{
    // Config
    ImGui::Begin("Control Panel");

    if (ImGui::BeginTable("PresetTable", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.6f);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Preset");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        static const std::vector<std::pair<PresetType, const char *>> preset_types = {
            {PresetType::Kinematic, "Calculated Kinematics"}, {PresetType::SolarSystem, "Solar Sytem"}};
        EnumCombo("##preset", universe_type, preset_types);

        ImGui::EndTable();
    }

    ImGui::Separator();

    switch (universe_type)
    {
    case phys::app::PresetType::Kinematic:
        tickKinematic(universe_main);
        break;
    case phys::app::PresetType::SolarSystem:
        tickSolarSystem(universe_main);
        break;
    }
    ImGui::End();
}
