#pragma once
#include "imgui.h"

namespace phys
{

class AppResources
{
  public:
    ImFont *font_regular;
    ImFont *font_small;
};

AppResources getAppResources();
void setAppResources(AppResources recources);

} // namespace phys
