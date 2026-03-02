
#pragma once
#include <SFML/Graphics.hpp>

namespace phys
{

class App
{
  public:
    App();
    void start();

  protected:
    // Init Opengl
    sf::RenderWindow app_window;
    // OpenGL Required

    // Window Variables
    sf::Clock delta_clock;

    virtual void tick();

  private:
    void _tick();
    void _render();

    void _pollEvents();
};

} // namespace phys
