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

class SceneWidget : protected TextureWidget
{
  public:
    using TextureWidget::TextureWidget;
    void update(Universe &universe, bool should_clear = true);

  private:
    unsigned int selected_body_id{0};
    std::pair<Body, Property> editing_pair{};
    phys::vec3d click_pos_world{};
};

class UniverseWidget : protected SceneWidget
{
  public:
    std::shared_ptr<Universe> universe;

    UniverseWidget();
    void update(bool should_clear = true);
};

class AlmagationWidget : public UniverseWidget
{
  public:
    std::vector<std::shared_ptr<Universe>> universes;
    std::vector<std::pair<float, Color>> properties;

    void resize(int amount);
    void resize_ColorSpectrum(int amount);
    void resize_TransperancyFade(int amount);
    void update();

  private:
    unsigned int selected_body_id{};
    Body editing_body{};
    phys::vec3d click_pos_world{};
};

} // namespace phys::app
