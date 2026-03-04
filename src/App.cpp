#include "App.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/WindowEnums.hpp"
#include "gl/ResourcesGl.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "tools/Debug.hpp"
#include "tools/Error.hpp"
#include <cassert>
#include <glad/glad.h>
#include <iostream>

using namespace phys;

App::App(sf::VideoMode videoMode, std::string title, std::uint32_t style, sf::State state, sf::ContextSettings settings)
    : app_window(videoMode, title, style, state, settings)
{
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam)
{
    auto isErrorStr = type == GL_DEBUG_TYPE_ERROR ? "**GL ERROR**" : "";
    // showMessageF("GL CALLBACK: {}, type = {}, severity = {}, message = {}", isErrorStr, type, severity, message);
}

void loadGlad()
{

    if (!gladLoadGLLoader((GLADloadproc)sf::Context::getFunction))
    {

        return;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
}

void App::start()
{
    loadGlad();
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
        if (!this->app_window.isOpen())
            break;
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
    ImGui::Begin("Debug Window");
    phys::updateDebug();
    ImGui::End();
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
