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
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::string sourceStr, typeStr, severityStr;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        sourceStr = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceStr = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceStr = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceStr = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sourceStr = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        sourceStr = "Other";
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        typeStr = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeStr = "Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeStr = "Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        typeStr = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        typeStr = "Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        typeStr = "Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        typeStr = "Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        typeStr = "Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        typeStr = "Other";
        break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        severityStr = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severityStr = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severityStr = "Low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severityStr = "Notification";
        break;
    }

    std::cerr << std::format("[GL {}] {} ({}): {} - {}", severityStr, typeStr, id, sourceStr, message) << std::endl;

    if (type == GL_DEBUG_TYPE_ERROR && severity == GL_DEBUG_SEVERITY_HIGH)
    {
        assert(false);
    }
}

void checkGLError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        printf("OpenGL Error: %d\n", err);
        // Trigger a breakpoint or exit
        __builtin_trap(); // Or assert(false);
        assert(false);
    }
}
void HardCheck(const char *site)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        // Log precisely WHERE this happened
        std::cerr << "CRITICAL STATE VIOLATION at [" << site << "]: " << err << std::endl;

// Use a platform-specific debug break to stop the spam immediately
#ifdef _MSC_VER
        __debugbreak();
#else
        __builtin_trap();
#endif
    }
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

    // Filter out notifications (they are usually noisy driver info)
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

    // Initial state to match expected frame buffer settings
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

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
        HardCheck("test");
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
