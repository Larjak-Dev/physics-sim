#include "App.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/WindowEnums.hpp"
#include "gl/ResourcesGl.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "tools/Error.hpp"
#include <cassert>
#include <glad/glad.h>
#include <iostream>

using namespace phys;

App::App(sf::VideoMode videoMode, std::string title, std::uint32_t style, sf::State state, sf::ContextSettings settings)
    : app_window(videoMode, title, style, state, settings)
{
}

void App::start()
{

    if (!gladLoadGLLoader((GLADloadproc)sf::Context::getFunction))
    {
        showMessage("Unable to load Glad!");
        return;
    }

    this->resourcesGl = std::make_shared<gl::ResourcesGl>();
    gl::setResourcesGL(this->resourcesGl);

    if (!ImGui::SFML::Init(this->app_window))
    {
        showMessage("Unable to init SFML-ImGui!");
        return;
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
