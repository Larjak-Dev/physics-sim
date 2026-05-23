#pragma once
#include "SFML/Window/GlResource.hpp"
#include "app/AppResources.hpp"
#include "core/universe/Camera.hpp"
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
    void update(Universe &universe);

  protected:
    AppContext &context;
    GlResources &resources_gl = this->context.resources_gl;
    AppResources &resources_app = this->context.resources_app;

    Renderer renderer{context};

    // Inputs
    void updateInputs(ImVec2 cursor, phys::Universe &universe, sf::RenderTexture &texture,
                      unsigned int &selected_body_id, phys::vec3d &mouse_world);

    // Rendering
    const Color BACKGROUND_COLOR = Color::Black;
    const gl::Texture &SKYBOX = resources_gl.stars;
    static constexpr float SKYBOX_TRANSPARENCY = 0.5f;

    static constexpr Color GRID_COLOR_SMALL = Color(0.5f, 0.5, 0.5, 1.0f);
    static constexpr Color GRID_COLOR_BIG = Color(1.0f, 1.0, 1.0, 1.0f);
    static constexpr float GRID_TRANSPARENCY = 1.0f;
    static constexpr float GRID_SCALE = 1.0f;

    static constexpr float FIELD_TRANSPARENCY = 0.5f;

    void updateTexture(Universe &universe);

    // GUI
    ImFont *VIEWCHILD_FONT = resources_app.font_small;
    static constexpr float VIEWCHILD_WIDTH = 160;
    static constexpr float VIEWCHILD_HEIGHT = 350;
    static constexpr Color VIEWCHILD_BACKGROUND = Color(0.2f, 0.2f, 0.2f, 0.5f);
    static constexpr float VIEWCHILD_ITEM_WIDTH = 150;

    const float RENDERING_HEIGHT = 100.0f;

    ImVec2 cursor{};
    unsigned int selected_body_id{0};
    phys::vec3d click_pos_world{};
    phys::Camera *cam;

    void updateViewportFloating(Universe &universe);
    void updateRenderingFloating(Universe &universe);
    void updateSelectionWin(Universe &universe);
    void updateBodiesWin(Universe &universe);
    void updateBodyPopup(Universe &universe);
    void updateWorldPopup(Universe &universe);

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
