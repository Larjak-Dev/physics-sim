#include "AppPhysics.hpp"
#include "SFML/Window/ContextSettings.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "universe/Camera.hpp"
#include "universe/Environment.hpp"
#include "universe/PhysicConfig.hpp"
#include <memory>

using namespace phys::app;

PhysicApp::PhysicApp(sf::ContextSettings settings)
    : App(sf::VideoMode({800, 500}), "PhysicApp", sf::Style::Titlebar | sf::Style::Close, sf::State::Windowed, settings)
{
    phys::Environment env{};
    phys::Body body{};
    phys::Property prop{};
    env.addBody(body, prop);

    this->universe = std::make_shared<Universe>();
    this->universe->env = std::make_shared<phys::EnvironmentActive>(env);
    this->universe->camera = std::make_shared<phys::Camera>(10.0);

    auto &force_config = this->universe->physicConfig.force_config;
    auto &step_config = this->universe->physicConfig.step_config;
    force_config.force_type = phys::ForceType::FreeFall;
    force_config.freefall_config.g = 9.81;
    step_config.step_type = phys::StepType::RK4;
    step_config.delta_time = 0.01;
    step_config.total_time = 10;
}

void PhysicApp::init()
{

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
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
        ImGuiID dock_left = DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.2f, nullptr, &dock_main);
        ImGuiID dock_right = DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.2f, nullptr, &dock_main);

        DockBuilderDockWindow("Preview", dock_main);
        DockBuilderDockWindow("Player", dock_main);
        DockBuilderDockWindow("Simulator", dock_main);

        DockBuilderDockWindow("Slides", dock_left);
        DockBuilderDockWindow("Control Panel", dock_right);
        DockBuilderDockWindow("Debug Panel", dock_right);
        DockBuilderDockWindow("Dear ImGui Demo", dock_right);
    }
}
