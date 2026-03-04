#include "AppPhysics.hpp"
#include "SFML/Window/ContextSettings.hpp"
#include "universe/Camera.hpp"
#include "universe/Environment.hpp"
#include <memory>

using namespace phys;

PhysicApp::PhysicApp(sf::ContextSettings settings)
    : App(sf::VideoMode({800, 500}), "PhysicApp", sf::Style::Titlebar | sf::Style::Close, sf::State::Windowed, settings)
{
    phys::Environment env{};
    phys::Body body{};
    phys::Property prop{};
    env.addBody(body, prop);

    this->universe = std::make_shared<Universe>();
    this->universe->env = std::make_shared<phys::EnvironmentActive>(env);
    this->universe->camera = std::make_shared<phys::Camera>(10.0);
}

void PhysicApp::tick()
{
    this->simulatorSlide.setUniverse(this->universe);
    this->simulatorSlide.tickContent();
}
