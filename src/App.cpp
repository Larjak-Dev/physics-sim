#include "App.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/WindowEnums.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include <cassert>

using namespace phys;

App::App() : app_window(sf::VideoMode({800, 480}), "PhysicsSim", sf::Style::Close, sf::State::Windowed)
{
}

void App::start()
{
    if (!ImGui::SFML::Init(this->app_window))
    {
        assert(false && "Failed to init ImGui SFML!");
    }
    while (this->app_window.isOpen())
    {
        _pollEvents();
        _tick();
        _render();
    }
    ImGui::SFML::Shutdown();
}

void App::_pollEvents()
{
    while (const std::optional event = this->app_window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(this->app_window, *event);
        if (event->is<sf::Event::Closed>())
        {
            this->app_window.close();
        }
    }
}

void App::_tick()
{
    ImGui::SFML::Update(this->app_window, this->delta_clock.restart());
    this->tick();
    ImGui::ShowDemoWindow();
}
void App::_render()
{
    this->app_window.clear(sf::Color(100, 100, 100));
    ImGui::SFML::Render(this->app_window);
    this->app_window.display();
}

void App::tick()
{
}
