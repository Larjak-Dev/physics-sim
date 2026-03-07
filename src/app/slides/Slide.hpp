#include "../../universe/Universe.hpp"
#pragma once

namespace phys::app
{
class Slide
{
  public:
    void setUniverse(std::shared_ptr<Universe> universe);

  protected:
    std::shared_ptr<phys::Universe> universe;
};
} // namespace phys::app
