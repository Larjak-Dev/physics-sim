#include "App.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/WindowEnums.hpp"
#include "core/tools/Debug.hpp"
#include "core/tools/Error.hpp"
#include <cassert>
#include <glad.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

using namespace phys::app;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam)
{
    auto isErrorStr = type == GL_DEBUG_TYPE_ERROR ? "**GL ERROR**" : "";
    std::cout << std::format("GL CALLBACK: {}, type = {}, severity = {}, /n message = {}", isErrorStr, type, severity,
                             message);
}

bool loadGlad()
{
    if (!gladLoadGLLoader((GLADloadproc)sf::Context::getFunction))
    {
        phys::showMessage(
            "Failed to initialize GLAD (OpenGL loader)! Your GPU or drivers might not support the requested "
            "OpenGL version.");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    return true;
}

#ifdef WIN32

#include <dwmapi.h> // Required for Dark Mode attribute

#pragma comment(lib, "dwmapi.lib") // Links the required Windows library

void setDarkMode(sf::WindowHandle handle)
{
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(handle,
                          20, // DWMWA_USE_IMMERSIVE_DARK_MODE (Windows 10 1903/1909)
                          &useDarkMode, sizeof(useDarkMode));

    // For Windows 11 and newer versions of Windows 10
    DwmSetWindowAttribute(handle,
                          20, // Some versions use 19, newer use 20
                          &useDarkMode, sizeof(useDarkMode));
}

#endif


App::App(sf::VideoMode videoMode, std::string title, std::uint32_t style, sf::State state, sf::ContextSettings settings)
    : app_window(videoMode, title, style, state, settings)
{
    app_window.setVerticalSyncEnabled(true);

    #ifdef WIN32
    setDarkMode(app_window.getNativeHandle());
    #endif


    if (!loadGlad())
    {
        this->app_window.close();
        return;
    }

    if (!ImGui::SFML::Init(this->app_window))
    {
        phys::showMessage("Unable to init SFML-ImGui!");
        return;
    }
}

void App::start()
{

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

    if (this->developer_mode)
    {
        ImGui::ShowDemoWindow();
        ImGui::Begin("Debug Panel");
        phys::updateDebug();
        ImGui::End();
    }
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
