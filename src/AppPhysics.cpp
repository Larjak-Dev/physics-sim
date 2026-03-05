#include "AppPhysics.hpp"
#include "SFML/Window/ContextSettings.hpp"
#include "universe/Camera.hpp"
#include "universe/Environment.hpp"
#include "universe/PhysicConfig.hpp"
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

    auto &force_config = this->universe->physicConfig.force_config;
    auto &step_config = this->universe->physicConfig.step_config;
    force_config.force_type = phys::ForceType::FreeFall;
    force_config.freefall_config.g = 9.81;
    step_config.step_type = phys::StepType::RK4;
    step_config.delta_time = 0.001;
    step_config.total_time = 10000;
}

void PhysicApp::tick()
{
    // this->simulatorSlide.setUniverse(this->universe);
    // this->simulatorSlide.tickContent();
    // this->simulatorSlide.tickRightBar();

    this->playerSlide.setUniverse(this->universe);
    this->playerSlide.tickContent();
    this->playerSlide.tickRightBar();
}
