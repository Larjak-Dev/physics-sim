
#pragma once
#include "../core/Units.hpp"
#include "../core/universe/Camera.hpp"
#include "GladWrap.hpp"
#include "app/AppResources.hpp"
#include "core/Environment.hpp"
#include "core/universe/Property.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <memory>
#include <vector>

namespace phys
{
class Universe;

struct Transform2D
{
    Camera camera{};
    vec2u res{};
    vec3d delta_transform;

    mat4f v;
    mat4f v_inverse;
    mat4f p;
    mat4f p_inverse;
    mat4f vp;
    mat4f vp_inverse;

    mat4f v_skybox;
    mat4f p_skybox;
    mat4f vp_skybox;

    Transform2D();
    void recalculate(const Camera &cam, vec2u res);

    vec3f apply(vec3d v);
    vec3d inverse(vec3f v);
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

    Renderer(AppContext &context);
    Renderer(const Renderer &other);
    Renderer &operator=(const Renderer &other);

    void activate(sf::RenderTarget &target);
    void deactivate();

    vec3d cordOnTargetToWorldCord(vec2f cord_on_target, const Camera &cam, double z, sf::RenderTarget &target);
    unsigned int cordOnTargetToBodyInWorld(vec2f cord_on_target, const Camera &cam, const EnvironmentBase &env,
                                           const std::vector<Property> &properties, sf::RenderTarget &target);
    void clear(Color background);

    void renderBodies(const EnvironmentBase &env, const std::vector<Property> &properties, const Camera &cam,
                      float transarency = 1.0f, Color color_addon = Color(0.0f, 0.0f, 0.0f, 0.0f));

    void renderBodiesAmalgamated(const std::vector<std::shared_ptr<Universe>> &universes,
                                 const std::vector<std::pair<float, Color>> &properties, const Camera &cam);

    void renderGrids(double scale, const Camera &cam, float transarency = 1.0f, double y = 0.0,
                     Color color_small = Color(1.0f, 1.0f, 1.0f, 1.0f),
                     Color color_big = Color(0.5f, 0.5f, 0.5f, 1.0f));

    void renderSkyBox(const gl::Texture &skybox, const Camera &cam, float transparency = 1.0f);

  private:
    AppContext &context;

    void renderGrid2D(double exponant, const Camera &cam, gl::ShaderMain &shader, double y = 0.0,
                      Color color = Color(1.0, 1.0, 1.0, 1.0), float transarency = 1.0f);
    void renderBodies2D(const EnvironmentBase &env, const std::vector<Property> &properties, const Camera &cam,
                        gl::ShaderMain &shader);

    // void render3D(sf::RenderTarget& target, sf::Vector2u size, const Environment& env, const Camera& cam);
};
} // namespace phys
