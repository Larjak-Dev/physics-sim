#pragma once
#include "../universe/Universe.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/System/Vector2.hpp"

namespace phys::panels
{

class PanelTexure
{
  public:
    sf::RenderTexture texture{{100, 100}};
    sf::Vector2u texture_size{100, 100};
    PanelTexure();
    void update();
};

class PanelScene : private PanelTexure
{
  public:
    using PanelTexure::PanelTexure;
    void update(Universe &universe);
};

} // namespace phys::panels
