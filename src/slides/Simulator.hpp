
#pragma once
#include "Slide.hpp"

namespace phys
{
class Simulator : public Slide
{
    void tickContent();
    void tickRightBar();
};
} // namespace phys
