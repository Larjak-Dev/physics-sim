#include "../../universe/Universe.hpp"
#pragma once

namespace phys::app
{
class Slide
{
  public:
    virtual void tickContent() = 0;
    virtual void tickRightBar() = 0;

    void setUniverse(std::shared_ptr<Universe> universe);

  protected:
    std::shared_ptr<phys::Universe> universe;
};
} // namespace phys::app
