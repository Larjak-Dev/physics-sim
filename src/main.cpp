#include "AppPhysics.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/WindowEnums.hpp"
#include <iostream>

int main()
{
    std::cout << "App started!\n";

    sf::ContextSettings settings;
    settings.majorVersion = 4;
    settings.minorVersion = 6;
    settings.attributeFlags = sf::ContextSettings::Default;

    phys::PhysicApp app(sf::VideoMode({800, 500}), "PhysicApp", sf::Style::Titlebar | sf::Style::Close,
                        sf::State::Windowed, settings);

    app.start();

    std::cout << "App closed!\n";
    return 0;
}
