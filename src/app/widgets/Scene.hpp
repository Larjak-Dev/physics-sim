#pragma once
#include "app/AppResources.hpp"
#include "core/universe/Universe.hpp"
#include "graphics/Renderer.hpp"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Vector2.hpp>

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
    SceneWidget(AppContext &context);
    void update(Universe &universe, bool should_clear = true);

  protected:
    AppContext &context;
    Renderer renderer{context};

    unsigned int selected_body_id{0};
    phys::vec3d click_pos_world{};

    void updateInputs(ImVec2 cursor, phys::Universe &universe, sf::RenderTexture &texture,
                      unsigned int &selected_body_id, phys::vec3d &mouse_world);

  private:
    std::pair<Body, Property> editing_pair{};
};

class UniverseWidget : protected SceneWidget
{
  public:
    std::shared_ptr<Universe> universe;

    UniverseWidget(AppContext &context);
    void update(bool should_clear = true);
};

class AlmagationWidget : public UniverseWidget
{
  public:
    std::vector<std::shared_ptr<Universe>> universes;
    std::vector<std::pair<float, Color>> properties;

    AlmagationWidget(AppContext &context);
    void resize(int amount);
    void resize_ColorSpectrum(int amount);
    void resize_TransperancyFade(int amount);
    void update();

  private:
    unsigned int selected_body_id{};
    Body editing_body{};
    phys::vec3d click_pos_world{};
};

class AnalyzeWidget : public SceneWidget
{
  public:
};

} // namespace phys::app
