#pragma once
#include "../../universe/Universe.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/System/Vector2.hpp"

namespace phys::app
{

class TextureWidget
{
  public:
    sf::RenderTexture texture{{100, 100}};
    sf::Vector2u texture_size{100, 100};
    TextureWidget();
    void update();
};

class SceneWidget : private TextureWidget
{
  public:
    using TextureWidget::TextureWidget;
    void update(Universe &universe);
};

class UniverseWidget : private SceneWidget
{
  public:
    std::shared_ptr<Universe> universe;

    UniverseWidget();
    void update();
};

class AlmagationWidget : private TextureWidget
{
  public:
    std::vector<std::shared_ptr<Universe>> universes;
    std::vector<std::pair<float, Color>> properties;

    void resize(int amount);
    void resize_ColorSpectrum(int amount);
    void resize_TransperancyFade(int amount);
    void update();
};

} // namespace phys::app
