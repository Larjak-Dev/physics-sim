#include "app/AppPhysics.hpp"
#include "core/tools/Error.hpp"
#include <iostream>

#include <nfd.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <SFML/Main.hpp>
#endif

int main()
{
    if (!std::filesystem::exists("assets") || !std::filesystem::is_directory("assets"))
    {
        phys::showMessage("Critical Error: 'assets' directory not found!\n"
                          "Please ensure you are running the application from the project root "
                          "or that the assets have been copied to the executable directory.");
        return 1;
    }

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
