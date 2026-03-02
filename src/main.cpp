#include "AppPhysics.hpp"
#include <iostream>

int main()
{
    std::cout << "App started!";
    phys::PhysicApp app;
    app.start();
    std::cout << "App closed!";
    return 0;
}
