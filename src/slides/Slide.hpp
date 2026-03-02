
#pragma once

namespace phys::slides
{
class Slide
{
    virtual void tickContent() = 0;
    virtual void tickRightBar() = 0;
};
} // namespace phys::slides
