
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
    sf::RenderTarget *target;

    void activate(sf::RenderTarget &target);
    void deactivate();

    void renderGrid(double exponant, const Camera &cam, const gl::Shader &shader,
                    Color color = Color(1.0, 1.0, 1.0, 1.0), float transarency = 1.0f);
    void render(const Environment &env, const Camera &cam, float transarency = 1.0f,
                Color color_addon = Color(0.0f, 0.0f, 0.0f, 0.0f));

  private:
    void render2D(const Environment &env, const Camera &cam, const gl::Shader &shader);

    // void render3D(sf::RenderTarget& target, sf::Vector2u size, const Environment& env, const Camera& cam);
};
} // namespace phys
