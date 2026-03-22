
#include "Units.hpp"
#include <imgui.h>

using namespace phys;

vec2f::vec2f(ImVec2 vec)
{
    this->x = vec.x;
    this->y = vec.y;
}
vec2f::operator ImVec2() const
{
    return ImVec2(this->x, this->y);
}
