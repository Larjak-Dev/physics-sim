
#pragma once
#include "../gl/GladWrap.hpp"
#include "Camera.hpp"
#include "Environment.hpp"
#include "tools/Units.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

namespace phys
{

struct Transform2D
{
    Camera camera{};
    vec2u res{};

    mat4d v;
    mat4d p;
    mat4d vp;

    mat4d p_inverse;
    mat4d vp_inverse;

    mat4f p_gl;
    mat4f v_skybox;
    mat4f p_skybox;
    mat4f vp_skybox;

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
    sf::RenderTarget *target{nullptr};
    gl::FrameBuffer frameBuffer{};
    gl::FrameBuffer frameBuffer_blur{};

    Renderer();
    Renderer(const Renderer &other);
    Renderer &operator=(const Renderer &other);

    void activate(sf::RenderTarget &target);
    void deactivate();

    vec3d cordOnTargetToWorldCord(vec2f cord_on_target, const Camera &cam, double z, sf::RenderTarget &target);
    unsigned int cordOnTargetToBodyInWorld(vec2f cord_on_target, const Camera &cam, Environment &env,
                                           sf::RenderTarget &target);
    void clear(Color background);

    void render(const Environment &env, const Camera &cam, float transarency = 1.0f,
                Color color_addon = Color(0.0f, 0.0f, 0.0f, 0.0f));
    void renderGrid(double exponant, const Camera &cam, float transarency = 1.0f,
                    Color color_addon = Color(0.0f, 0.0f, 0.0f, 0.0f));

    void renderSkyBox(gl::Texture &skybox, const Camera &cam, float transparency = 1.0f);

  private:
    void renderGrid2D(double exponant, const Camera &cam, gl::ShaderMain &shader,
                      Color color = Color(1.0, 1.0, 1.0, 1.0), float transarency = 1.0f);
    void render2D(const Environment &env, const Camera &cam, gl::ShaderMain &shader);

    // void render3D(sf::RenderTarget& target, sf::Vector2u size, const Environment& env, const Camera& cam);
};
} // namespace phys
