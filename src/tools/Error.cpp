#include "Error.hpp"
#include <cstdlib>
#include <iostream>

using namespace phys;

void phys::showMessage(const char *msg)
{
    std::cout << msg;
    std::system(msg);
}
