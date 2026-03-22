
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
}

void Editor::tickContent()
{
    ImGui::Begin("Editor", nullptr);
    this->reviewPanel.update(*this->universe, true);
    ImGui::End();
}

void Editor::tickKinematic(std::shared_ptr<Universe> &universe_main)
{
    static const std::vector<std::pair<phys::ForceType, const char *>> force_types = {
        {phys::ForceType::FreeFall, "Free Fall"}, {phys::ForceType::Newtonian, "Satelite Orbit"}};
    EnumCombo("Kinematic Environment", this->kinematic_config.type, force_types);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2, 0.2, 0.2, 0.5));
    ImGui::BeginChild("Input", ImVec2(0, 100), ImGuiChildFlags_Borders);
    switch (this->kinematic_config.type)
    {
    case phys::ForceType::FreeFall:
    {
        ImGui::InputDouble("Acceleration", &kinematic_config.acceleration, 0, 0, "%.4e m/s^2");
        ImGui::InputDouble("Total Time", &kinematic_config.time_fall, 0, 0, "%.4e s");
    }
    break;
    case phys::ForceType::Newtonian:
    {
        ImGui::InputDouble("G", &kinematic_config.G, 0, 0, "%.6e F/kg");
        ImGui::InputDouble("Mass Planet", &kinematic_config.mass_planet, 0, 0, "%.4e");
        ImGui::InputDouble("Distance", &kinematic_config.distance, 0, 0, "%.4e m");
        ImGui::InputDouble("Total Time", &kinematic_config.time_satelite, 0, 0, "%.4e s");
    }
    break;
    default:
        break;
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (ImGui::Button("Configure"))
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

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2, 0.2, 0.2, 0.5));
    ImGui::BeginChild("Input", ImVec2(0, 100), ImGuiChildFlags_Borders);
    ImGui::InputInt("Year", &this->planet_api.year_1);
    ImGui::InputInt("Month", &this->planet_api.month_1);
    ImGui::InputInt("Day", &this->planet_api.day_1);
    ImGui::InputDouble("Total Time (Days):", &this->planet_api.total_days);
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (ImGui::Button("Configure"))
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

    static const std::vector<std::pair<PresetType, const char *>> preset_types = {
        {PresetType::Kinematic, "Calculated Kinematics"}, {PresetType::SolarSystem, "Solar Sytem"}};
    EnumCombo("Preset ", universe_type, preset_types);

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
