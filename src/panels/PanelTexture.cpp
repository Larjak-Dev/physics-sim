#include "PanelTexure.hpp"
#include "SFML/System/Vector2.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "universe/Universe.hpp"
#include <cmath>

using namespace phys::panels;

PanelTexure::PanelTexure()
{
}

void PanelTexure::update()
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

#include "../tools/Debug.hpp"
void PanelScene::update(phys::Universe &universe)
{
    auto cursor = ImGui::GetCursorPos();

    PanelTexure::update();

    ImGui::SetCursorPos(cursor);
    ImGui::InvisibleButton("##viewport_drag", ImGui::GetItemRectSize());
    if (ImGui::IsItemActive())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {

            static vec2f prevPos;
            auto newPos = vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left));
            auto delta = newPos - prevPos;
            prevPos = newPos;

            auto delta_scene = 2.0 * vec2d(delta) / vec2d(vec2u(texture.getSize()));
            auto delta_world = universe.renderer.transform2D.p_inverse * vec4d(delta_scene.x, delta_scene.y, 0.0, 1.0);

            auto delta_crossX = universe.camera->getCrossX() * -delta_world.x;
            auto delta_crossY = universe.camera->getCrossY() * delta_world.y;
            auto delta_cam = delta_crossX + delta_crossY;

            universe.camera->center += delta_cam;
        }
    }
    auto center = universe.camera->center;
    phys::showDebugF("Camera {}, {}, {}", center.x, center.y, center.z);

    universe.renderer.render(this->texture, universe.env, universe.camera);
}
