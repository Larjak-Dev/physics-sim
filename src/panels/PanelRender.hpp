#pragma once
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/System/Vector2.hpp"

namespace phys::panels
{

class PanelRender
{
  public:
    sf::RenderTexture texture{{100, 100}};
    sf::Vector2u texture_size{100, 100};
    PanelRender();
    void tick();
    void update();
};

} // namespace phys::panels
