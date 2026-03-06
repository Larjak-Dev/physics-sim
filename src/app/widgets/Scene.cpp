#include "Scene.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/System/Vector2.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "tools/Units.hpp"
#include "universe/Environment.hpp"
#include "universe/Universe.hpp"
#include <cmath>
#include <ranges>

using namespace phys::app;

TextureWidget::TextureWidget()
{
}

void TextureWidget::update()
{
    auto size = ImGui::GetContentRegionAvail();

    ImGui::Image(this->texture);

    auto sizeSF =
        sf::Vector2u(static_cast<unsigned int>(std::floor(size.x)), static_cast<unsigned int>(std::floor(size.y)));
    if (sizeSF != this->texture_size)
    {
        bool isSuccesful = this->texture.resize(sizeSF);
        if (isSuccesful)
        {
            this->texture_size = sizeSF;
        }
    }
}

#include "../../tools/Debug.hpp"
void updateDragging(ImVec2 cursor, const phys::Universe &universe, sf::RenderTexture &texture)
{
    using namespace phys;
    ImGui::SetCursorPos(cursor);
    ImGui::InvisibleButton("##viewport_drag", ImGui::GetItemRectSize());
    if (ImGui::IsItemActive())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {

            auto mouse_delta = ImGui::GetIO().MouseDelta;
            auto delta = vec2f(mouse_delta.x, mouse_delta.y);

            auto delta_scene = 2.0 * vec2d(delta) / vec2d(vec2u(texture.getSize()));
            auto delta_world = universe.renderer.transform2D.p_inverse * vec4d(delta_scene.x, delta_scene.y, 0.0, 1.0);

            auto delta_crossX = universe.camera->getCrossX() * -delta_world.x;
            auto delta_crossY = universe.camera->getCrossY() * delta_world.y;
            auto delta_cam = delta_crossX + delta_crossY;

            universe.camera->center += delta_cam;
        }
    }
}

void SceneWidget::update(phys::Universe &universe)
{
    if (!universe.env)
    {
        ImGui::Text("Uninitialised Environment!");
        return;
    }
    if (!universe.camera)
    {
        ImGui::Text("Uninitialised Camera!");
        return;
    }

    auto cursor = ImGui::GetCursorPos();

    TextureWidget::update();
    updateDragging(cursor, universe, texture);

    auto center = universe.camera->center;
    phys::showDebugF("Camera {}, {}, {}", center.x, center.y, center.z);

    this->texture.clear();
    universe.renderer.activate(this->texture);
    universe.renderer.render(static_cast<Environment>(*universe.env), *universe.camera);
    universe.renderer.deactivate();
}

UniverseWidget::UniverseWidget()
{
    this->universe = std::make_shared<Universe>();
}
void UniverseWidget::update()
{

    if (!universe)
    {
        ImGui::Text("Uninitialised Universe");
        return;
    }
    SceneWidget::update(*this->universe);
}

void AlmagationWidget::resize(int amount)
{
    this->universes.resize(amount);
    this->properties.resize(amount);

    for (auto &universe : this->universes)
    {
        if (!universe)
        {
            universe = std::make_shared<Universe>();
        }
    }
}
#include <glm/common.hpp>

phys::Color hueToRGB(float f)
{
    phys::vec3f rgb =
        glm::clamp(glm::abs(glm::mod(f * 6.0f + phys::vec3f(0.0f, 4.0f, 2.0), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f);
    return phys::Color(rgb.r, rgb.g, rgb.b, 1.0f);
}

void AlmagationWidget::resize_ColorSpectrum(int amount)
{
    resize(amount);
    int size = static_cast<int>(this->properties.size());
    int index = 0;
    for (auto &[transparency, color] : this->properties)
    {
        transparency = 1.0f;
        color = hueToRGB(static_cast<float>(index) / static_cast<float>(size));
        index++;
    }
}
void AlmagationWidget::resize_TransperancyFade(int amount)
{
    resize(amount);

    int size = static_cast<int>(this->properties.size());
    int index = 0;
    for (auto &[transparency, color] : this->properties)
    {
        transparency = (1 - static_cast<float>(index) / static_cast<float>(size));
        color = Color(0.0f, 0.0f, 0.0f, 0.0f);
        index++;
    }
}

void AlmagationWidget::update()
{
    if (universes.size() == 0 || !universes[0])
    {
        ImGui::Text("Uninitialised Universe");
        return;
    }
    if (!universes[0]->env)
    {
        ImGui::Text("Uninitialised Environment!");
        return;
    }
    if (!universes[0]->camera)
    {
        ImGui::Text("Uninitialised Camera!");
        return;
    }
    auto cursor = ImGui::GetCursorPos();

    TextureWidget::update();
    updateDragging(cursor, *this->universes[0], texture);

    this->texture.clear();
    this->universes[0]->renderer.activate(this->texture);
    for (auto &&[i, universe] : std::views::enumerate(this->universes))
    {
        if (!universe || !universe->env)
            continue;
        this->universes[0]->renderer.render(static_cast<Environment>(*universe->env), *this->universes[0]->camera,
                                            this->properties[i].first, this->properties[i].second);
    }
    this->universes[0]->renderer.deactivate();
}
