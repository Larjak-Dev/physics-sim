#include "Renderer.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/System/Vector2.hpp"
#include "gl/GladWrap.hpp"
#include "gl/ResourcesGl.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"
#include "tools/Debug.hpp"
#include "tools/Units.hpp"
#include "universe/Environment.hpp"
#include <cmath>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include <ranges>

#include "../tools/Debug.hpp"

using namespace phys;

Renderer::Renderer()
{
}

Renderer::Renderer(const Renderer &other)
{
    this->transform2D = other.transform2D;
    this->target = other.target;
    if (other.target)
    {
        this->frameBuffer.resize(other.target->getSize());
    }
}

Renderer &Renderer::operator=(const Renderer &other)
{
    if (this == &other)
        return *this;
    this->transform2D = other.transform2D;
    this->target = other.target;
    if (other.target)
    {
        this->frameBuffer.resize(other.target->getSize());
    }
    return *this;
}

Transform2D::Transform2D()
{
    this->p_gl = mat4f(1.0f);
}

void Transform2D::recalculate(const Camera &cam, vec2u res)
{
    bool changed = false;
    if (this->camera != cam)
    {
        auto eye = cam.getEye();
        this->v = glm::lookAt(eye, cam.center, vec3d(0.0, 1.0, 0.0));
        changed = true;
    }
    if (this->res != res || this->camera != cam)
    {
        vec2d resd = vec2d(res);
        auto sv = cam.distance * resd / 300.0;
        this->p = glm::ortho(-sv.x / 2.0, sv.x / 2.0, -sv.y / 2.0, sv.y / 2.0, cam.distance - sv.x / 2.0,
                             cam.distance + sv.y / 2.0);
        this->p_inverse = glm::inverse(this->p);
        changed = true;
    }
    if (changed)
    {
        this->vp = this->p * this->v;
        this->vp_inverse = glm::inverse(this->vp);
    }
    this->camera = cam;
    this->res = res;
}

vec3d Renderer::cordOnTargetToWorldCord(vec2f cord_on_target, const Camera &cam, double z, sf::RenderTarget &target)
{

    this->transform2D.recalculate(cam, target.getSize());
    auto vp_inverse = this->transform2D.vp_inverse;

    vec2f screen_size = vec2f(vec2u(target.getSize()));
    vec2f gl_cord = 2.0f * cord_on_target / screen_size - 1.0f;
    vec4d world_cord = vp_inverse * vec4d(gl_cord.x, -gl_cord.y, z, 1.0);
    return vec3d(world_cord);
}

unsigned int Renderer::cordOnTargetToBodyInWorld(vec2f cord_on_target, const Camera &cam, Environment &env,
                                                 sf::RenderTarget &target)
{
    auto ray_start = cordOnTargetToWorldCord(cord_on_target, cam, -1.0, target);
    auto ray_end = cordOnTargetToWorldCord(cord_on_target, cam, 1.0, target);
    auto ray_delta_norm = glm::normalize(ray_end - ray_start);

    double selected_distance = std::numeric_limits<double>::max();
    Body *selected_body = nullptr;
    for (auto &&[body, property] : std::views::zip(env.bodies, env.properties))
    {
        double radius = property.size.x;
        if (cam.is_fixed_body_size)
        {
            radius = 1 * cam.distance * cam.fixed_size / 12.0;
        }
        else if (cam.is_scaled_body_size)
        {
            radius = property.size.x * cam.body_scale;
        }
        double distance = 0.0f;
        if (glm::intersectRaySphere(ray_start, ray_delta_norm, body.pos, radius * radius, distance) &&
            distance < selected_distance)
        {
            selected_body = &body;
            selected_distance = distance;
        }
    }
    if (selected_body)
    {
        return selected_body->id;
    }
    return 0;
}

void Renderer::clear(Color background)
{
    this->frameBuffer.texture_1.clear(background);
    this->frameBuffer.texture_2.clear(Color::Transparent);
    this->frameBuffer.texture_3.clear(Color::Transparent);
    this->frameBuffer.texture_4.clear(Color::Transparent);
}

void Renderer::render(const Environment &env, const Camera &cam, float transparency, Color color_addon)
{
    assert(this->target);
    auto &target = *this->target;

    auto &shader = gl::getResourcesGL()->mainShader;
    shader.setTransparency(transparency);
    shader.setColorExt(color_addon);

    this->frameBuffer.resize(target.getSize());
    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    render2D(env, cam, shader);

    /// Blur
    // Horizontal Blur
    auto &shader_blur = gl::getResourcesGL()->shader_blur;
    this->frameBuffer.activate(gl::FrameBuffer::Slot_3, 0, 0, 0);
    shader_blur.setTexture(this->frameBuffer.texture_1);
    shader_blur.setIsVertical(false);
    shader_blur.use();
    gl::getResourcesGL()->quad.render();

    // Vertical Blur
    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, 0, 0, 0);
    shader_blur.setTexture(this->frameBuffer.texture_3);
    shader_blur.setIsVertical(true);
    shader_blur.use();
    gl::getResourcesGL()->quad.render();

    this->target->setActive(true);
    shader.setMatrixM(mat4f(1.0f));
    shader.setColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    shader.use();
    this->frameBuffer.texture_1.bindUnit(0);
    gl::getResourcesGL()->quad.render();
}

void Renderer::renderGrid(double scale, const Camera &cam, float transparency, Color color)
{
    auto &shader = gl::getResourcesGL()->mainShader;
    double exponant_1 = std::floor(std::log10(cam.distance * 0.6));
    double exponant_2 = std::floor(std::log10(cam.distance) + 1);
    phys::showDebugF("Exponent: {}", exponant_1);
    this->frameBuffer.resize(target->getSize());
    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    this->renderGrid2D(exponant_1, cam, shader, Color(0.5f, 0.5f, 0.5f), transparency);
    this->renderGrid2D(exponant_2, cam, shader, Color(1.0f, 1.0f, 1.0f), transparency);
}

mat4f getModelTransform(const vec4d pos_world, const vec4d size_world, const mat4d &vp_world)
{
    auto sum_world = pos_world + size_world;

    auto pos_scene = vp_world * pos_world;
    auto sum_scene = vp_world * sum_world;
    auto size_scene = sum_scene - pos_scene;

    mat4f model = mat4f(1.0f);
    model = glm::translate(model, vec3f(pos_scene));

    model = glm::scale(model, vec3f(size_scene));
    return model;
}

void Renderer::renderGrid2D(double exponant, const Camera &cam, gl::ShaderMain &shader, Color color, float transparency)
{
    assert(this->target);
    auto &target = *this->target;

    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);
    shader.use();

    mat4f vp_gl = mat4f(1.0f);
    shader.setMatrixVP(vp_gl);

    shader.setColor(color);
    shader.setColorExt(color);
    shader.setTransparency(transparency);

    this->transform2D.recalculate(cam, target.getSize());
    auto vp_world = this->transform2D.vp;

    // Grid Rendering
    auto &vertexArrayGrid = gl::getResourcesGL()->grid;
    const double scale = std::pow(10, exponant);

    const auto amountGrid = gl::getResourcesGL()->gridAmount;
    const auto center_world = vec4d(glm::round(cam.center / scale) * scale, 1.0f);
    const auto size_world = vec4d{scale * amountGrid / 2.0, scale * amountGrid / 2.0, scale * amountGrid / 2.0, 0.0};

    auto model = getModelTransform(center_world, size_world, vp_world);
    shader.setMatrixM(model);

    vertexArrayGrid.renderLines();
}

void Renderer::render2D(const Environment &env, const Camera &cam, gl::ShaderMain &shader)
{
    assert(env.properties.size() >= env.bodies.size());

    assert(this->target);
    auto &target = *this->target;

    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);

    shader.use();

    mat4f vp_gl = mat4f(1.0f);
    shader.setMatrixVP(vp_gl);

    // Pre calculate view project
    this->transform2D.recalculate(cam, target.getSize());
    auto vp_world = this->transform2D.vp;

    // Render Bodies
    auto &vertexArray = gl::getResourcesGL()->sphere;
    for (auto [index, body] : std::views::enumerate(env.bodies))
    {
        // Model Configuring
        auto pos_world = vec4d(body.pos, 1.0f);
        auto size_world = vec4d(env.properties[index].size, 0.0f);
        if (cam.is_fixed_body_size)
        {
            size_world = vec4d(1.0, 1.0, 1.0, 0.0) * cam.distance * cam.fixed_size / 12.0;
        }
        else if (cam.is_scaled_body_size)
        {
            size_world = vec4d(env.properties[index].size, 0.0f) * cam.body_scale;
        }

        auto model = getModelTransform(pos_world, size_world, vp_world);
        shader.setMatrixM(model);

        // Color + Texture
        auto color = env.properties[index].color;
        if (env.properties[index].texture && cam.is_render_textures)
        {
            env.properties[index].texture->bindUnit(0);
            shader.setColor({0, 0, 0, 0});
        }
        else
        {
            gl::getResourcesGL()->default_tex.bindUnit(0);
            shader.setColor(color);
        }

        vertexArray.render();
    }
}

void Renderer::activate(sf::RenderTarget &target)
{
    assert(!this->target);
    this->target = &target;
    if (!target.setActive(true))
    {
        return;
    }
    target.pushGLStates();
}

void Renderer::deactivate()
{
    assert(this->target);
    glUseProgram(0);

    target->popGLStates();
    this->target = nullptr;
}
