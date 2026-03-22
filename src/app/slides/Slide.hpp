#include "app/AppResources.hpp"
#include "core/universe/Universe.hpp"
#pragma once

namespace phys::app
{
class Slide
{
  public:
    Slide(AppContext &context);
    void setUniverse(std::shared_ptr<Universe> universe);

  protected:
    std::shared_ptr<phys::Universe> universe;
    AppContext &context;
};
} // namespace phys::app
