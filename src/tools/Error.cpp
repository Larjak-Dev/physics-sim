#include "Error.hpp"
#include <cstdlib>
#include <iostream>

using namespace phys;

void phys::showMessage(const char *msg)
{
    std::cout << msg;
    auto cmd = std::format("notify-send -u critical \"PhysicsSim: {}\"", msg);
    std::system(cmd.c_str());
}
