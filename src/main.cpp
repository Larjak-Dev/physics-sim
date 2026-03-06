#include "app/AppPhysics.hpp"
#include <iostream>

int main()
{
    std::cout << "App started!\n";

    sf::ContextSettings settings;
    settings.majorVersion = 4;
    settings.minorVersion = 6;
    settings.attributeFlags = sf::ContextSettings::Default;

    phys::app::PhysicApp app(settings);

    app.start();

    std::cout << "App closed!\n";
    return 0;
}
