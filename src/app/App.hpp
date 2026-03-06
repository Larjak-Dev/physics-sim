
#pragma once
#include "../gl/ResourcesGl.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <cstdint>

namespace phys::app
{

class App
{
  public:
    App(sf::VideoMode videoMode, std::string title, std::uint32_t style, sf::State state, sf::ContextSettings settings);
    void start();

  protected:
    // Init Opengl
    sf::RenderWindow app_window;
    // OpenGL Required
    std::shared_ptr<gl::ResourcesGl> resourcesGl;

    // Window Variables
    sf::Clock delta_clock;

    virtual void init();
    virtual void tick();

  private:
    void _tick();
    void _render();

    void _pollEvents();
};

} // namespace phys::app
