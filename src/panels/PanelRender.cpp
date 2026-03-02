#include "PanelRender.hpp"
#include "SFML/System/Vector2.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include <cmath>

using namespace phys::panels;

PanelRender::PanelRender()
{
}

void PanelRender::update()
{
    auto size = ImGui::GetWindowSize();

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
