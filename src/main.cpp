#include "AppPhysics.hpp"
#include <iostream>

int main()
{
    std::cout << "App started!\n";
    phys::PhysicApp app;
    app.start();
    std::cout << "App closed!\n";
    return 0;
}
