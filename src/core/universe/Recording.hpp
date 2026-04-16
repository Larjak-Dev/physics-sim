#pragma once
#include "Universe.hpp"
#include "physics/Simulator.hpp"
#include <memory>
#include <vector>

namespace phys
{

struct AppRecording
{
    std::shared_ptr<phys::Recording> physics_recording;
    std::shared_ptr<Universe> universe;
    std::vector<phys::EnvironmentBase> frames_kinematic;

    AppRecording(std::shared_ptr<Universe> u) : universe(u)
    {
        physics_recording = std::make_shared<phys::Recording>();
    }
};

} // namespace phys
