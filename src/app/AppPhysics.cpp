#include "AppPhysics.hpp"
#include "app/AppResources.hpp"
#include "core/universe/PhysicConfig.hpp"
#include "physics/Kinematics.hpp"
#include <ImGuiUtils.h>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_internal.h>

using namespace phys::app;

PhysicApp::PhysicApp(sf::ContextSettings settings)
    : App(sf::VideoMode({1400, 800}), "PhysicApp", sf::Style::Default, sf::State::Windowed, settings)
{
    // ImGui setup
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    auto &resources = this->appContext.resources_app;

    resources.font_regular = io.Fonts->AddFontFromFileTTF("assets/inter.ttf", 16.0f);
    if (resources.font_regular)
    {
        io.FontDefault = resources.font_regular;
    }

    resources.font_small = io.Fonts->AddFontFromFileTTF("assets/inter.ttf", 14.0f);
    if (!resources.font_small)
    {
        resources.font_small = io.FontDefault;
    }

    ImGui::SetupImGuiStyle(true, 1.0f);
    ImGui::SFML::UpdateFontTexture();

    // Variables
    auto config = phys::createPerfectSatelite(1.0, 1.0, 10, 2.0);
    this->universe = std::make_shared<Universe>(phys::createUniverse(config));
}

void PhysicApp::tick()
{

    auto dock_id = ImGui::DockSpaceOverViewport();
    buildDock(dock_id);

    ImGui::Begin("Slides");
    if (ImGui::Button("Editor"))
    {
        this->selected_slide = SlideType::Editor;
    }
    if (ImGui::Button("Simulator"))
    {
        this->selected_slide = SlideType::Simulator;
    }
    if (ImGui::Button("Player"))
    {
        this->selected_slide = SlideType::Player;
    }
    ImGui::End();

    switch (this->selected_slide)
    {
    case SlideType::Editor:
        this->editor_slide.setUniverse(this->universe);
        this->editor_slide.tickContent();
        this->editor_slide.tickRightBar(this->universe);
        break;
    case SlideType::Simulator:
        this->simulator_slide.setUniverse(this->universe);
        this->simulator_slide.tickContent();
        this->simulator_slide.tickRightBar();
        break;
    case SlideType::Player:

        this->player_slide.setUniverse(this->universe);
        this->player_slide.tickContent();
        this->player_slide.tickRightBar();
        break;
    }
}

void PhysicApp::buildDock(int dock_id)
{
    static bool first = true;
    if (first)
    {
        first = false;

        using namespace ImGui;
        DockBuilderRemoveNode(dock_id);
        DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);
        DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_main = dock_id;
        ImGuiID dock_left = DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.1f, nullptr, &dock_main);
        ImGuiID dock_right = DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.2f, nullptr, &dock_main);
        ImGuiID dock_right_down = DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.3f, nullptr, &dock_right);

        DockBuilderDockWindow("Preview", dock_main);
        DockBuilderDockWindow("Player", dock_main);
        DockBuilderDockWindow("Simulator", dock_main);
        DockBuilderDockWindow("Editor", dock_main);

        DockBuilderDockWindow("Slides", dock_left);

        DockBuilderDockWindow("Control Panel", dock_right);
        DockBuilderDockWindow("Debug Panel", dock_right);
        DockBuilderDockWindow("Dear ImGui Demo", dock_right);

        DockBuilderDockWindow("Bodies", dock_right_down);
        DockBuilderDockWindow("Selection", dock_right_down);
    }
}
