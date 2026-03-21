#include "AppPhysics.hpp"
#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/WindowEnums.hpp"
#include "app/AppResources.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "physics/Kinematics.hpp"
#include "universe/PhysicConfig.hpp"
#include <ImGuiUtils.h>

using namespace phys::app;

PhysicApp::PhysicApp(sf::ContextSettings settings)
    : App(sf::VideoMode({1400, 800}), "PhysicApp", sf::Style::Default, sf::State::Windowed, settings)
{
    auto config = phys::createPerfectSatelite(1.0, 1.0, 10, 2.0);
    this->universe = std::make_shared<Universe>(phys::createUniverse(config));
}

void PhysicApp::init()
{
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    auto resources = this->resources;
    // Load JetBrains Mono (Size 18-20 is sweet spot for high-DPI)
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

    phys::setAppResources(resources);

    // Apply the spectrum theme style
    ImGui::SetupImGuiStyle(true, 1.0f);

    // CRITICAL: Re-bake the font atlas texture for the GPU
    ImGui::SFML::UpdateFontTexture();
}

void PhysicApp::tick()
{

    auto dock_id = ImGui::DockSpaceOverViewport();
    buildDock(dock_id);

    ImGui::Begin("Slides");
    if (ImGui::Button("Editor"))
    {
        this->selectedSlide = SlideType::Editor;
    }
    if (ImGui::Button("Simulator"))
    {
        this->selectedSlide = SlideType::Simulator;
    }
    if (ImGui::Button("Player"))
    {
        this->selectedSlide = SlideType::Player;
    }
    ImGui::End();

    switch (this->selectedSlide)
    {
    case SlideType::Editor:
        this->editorSlide.setUniverse(this->universe);
        this->editorSlide.tickContent();
        this->editorSlide.tickRightBar(this->universe);
        break;
    case SlideType::Simulator:
        this->simulatorSlide.setUniverse(this->universe);
        this->simulatorSlide.tickContent();
        this->simulatorSlide.tickRightBar();
        break;
    case SlideType::Player:

        this->playerSlide.setUniverse(this->universe);
        this->playerSlide.tickContent();
        this->playerSlide.tickRightBar();
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
