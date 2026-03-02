
#pragma once
#include "Camera.hpp"
#include "Environment.hpp"
#include <memory>

namespace phys
{
class RenderConfig
{

    void render(const std::shared_ptr<EnvironmentActive> env, const std::shared_ptr<Camera> cam);
};
} // namespace phys
