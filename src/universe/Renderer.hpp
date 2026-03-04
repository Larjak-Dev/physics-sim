
#pragma once
#include "../gl/GladWrap.hpp"
#include "Camera.hpp"
#include "Environment.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <memory>

namespace phys
{

struct Transform2D
{
    Camera camera;
    vec2u res;

    mat4d v;
    mat4d p;
    mat4d vp;

    mat4d p_inverse;
    mat4d vp_inverse;

    mat4f p_gl;

    Transform2D();
    void recalculate(const Camera &cam, vec2u res);
};

////////////
/// Renderer
////////////

class Renderer
{
  public:
    Transform2D transform2D;

    void render(sf::RenderTarget &target, const std::shared_ptr<EnvironmentActive> env,
                const std::shared_ptr<Camera> cam);
    void render(sf::RenderTarget &target, const Environment &env, const Camera &cam);

  private:
    void render2D(sf::RenderTarget &target, const Environment &env, const Camera &cam, const gl::Shader &shader);

    // void render3D(sf::RenderTarget& target, sf::Vector2u size, const Environment& env, const Camera& cam);
};
} // namespace phys
