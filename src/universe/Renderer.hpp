
#pragma once
#include "Camera.hpp"
#include "Environment.hpp"
#include "SFML/System/Vector2.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <glm/matrix.hpp>
#include <memory>

namespace phys
{

using mat4d = glm::mat<4, 4, double>;
using mat4f = glm::mat<4, 4, float>;

struct TransformWorld
{
    mat4d v;
    mat4d p;
    mat4d vp;
};

TransformWorld createTransformWorld2D(const Camera &cam, vec2u res);

class RenderConfig
{
  public:
    void render(sf::RenderTarget &target, sf::Vector2u size, const std::shared_ptr<EnvironmentActive> env,
                const std::shared_ptr<Camera> cam);

  private:
    void render2D(sf::RenderTarget &target, sf::Vector2u size, const Environment &env, const Camera &cam);

    // void render3D(sf::RenderTarget& target, sf::Vector2u size, const Environment& env, const Camera& cam);
};
} // namespace phys
