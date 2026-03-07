
#include "Editor.hpp"
#include "../../physics/Kinematics.hpp"
#include "../widgets/extra.hpp"
#include "imgui.h"
#include "tools/Debug.hpp"
#include "tools/Error.hpp"
#include "universe/PhysicConfig.hpp"
#include <utility>
using namespace phys::app;

void Editor::tickContent()
{
    ImGui::Begin("Editor", nullptr);
    this->reviewPanel.update(*this->universe);
    ImGui::End();
}

void Editor::tickRightBar(std::shared_ptr<Universe> &universe_main)
{
    // Config
    ImGui::Begin("Control Panel");

    static const std::vector<std::pair<phys::ForceType, const char *>> force_types = {
        {phys::ForceType::FreeFall, "Free Fall"}, {phys::ForceType::Newtonian, "Satelite Orbit"}};
    EnumCombo("Step Method", this->config.type, force_types);

    ImGui::BeginChild("Input", ImVec2(0, 100));
    switch (this->config.type)
    {
    case phys::ForceType::FreeFall:
    {
        ImGui::InputDouble("Acceleration", &config.acceleration, 0, 0, "%.4e");
        ImGui::InputDouble("Total Time", &config.time_fall, 0, 0, "%.4e");
    }
    break;
    case phys::ForceType::Newtonian:
    {
        ImGui::InputDouble("G", &config.G, 0, 0, "%.6e");
        ImGui::InputDouble("Mass Planet", &config.mass_planet, 0, 0, "%.4e");
        ImGui::InputDouble("Distance", &config.distance, 0, 0, "%.4e");
        ImGui::InputDouble("Total Time", &config.time_satelite, 0, 0, "%.4e");
    }
    break;
    default:
        break;
    }
    ImGui::EndChild();

    if (ImGui::Button("Configure"))
    {
        if (this->config.type == ForceType::Null)
        {
            showMessage("Unvalid kinematic config");
        }
        else
        {
            auto config_uni = phys::createKinematicScenario(this->config);
            universe_main = std::make_shared<Universe>(createUniverse(config_uni));
        }
    }

    ImGui::End();
}
