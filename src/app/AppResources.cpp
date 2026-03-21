#include "AppResources.hpp"
using namespace phys;

AppResources resources_;

AppResources phys::getAppResources()
{
    return resources_;
}
void phys::setAppResources(AppResources resources)
{
    resources_ = resources;
}
