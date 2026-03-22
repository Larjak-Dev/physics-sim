#include "Renderer.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/System/Vector2.hpp"
#include "gl/GladWrap.hpp"
#include "gl/ResourcesGl.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/matrix.hpp"
#include "tools/Debug.hpp"
#include "tools/Units.hpp"
#include "universe/Camera.hpp"
#include "universe/Environment.hpp"
#include <cmath>
#include <glad.h>
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
}

void Transform2D::recalculate(const Camera &cam, vec2u res)
{
    bool changed = false;
    if (this->camera != cam)
    {
        auto eye = static_cast<vec3f>(cam.getEye() - cam.center);

        this->delta_transform = -cam.center;

        // The camera's Local Up is determined by rotating the global Y axis
        // (Because you pull the camera back along the Z axis)
        vec3f up = cam.getRotationMatrix() * vec4f(0.0f, 1.0f, 0.0f, 0.0f);

        this->v = glm::lookAt(eye, vec3f(0.0, 0.0, 0.0), up);
        this->v_inverse = glm::inverse(this->v);
        changed = true;

        // SkyBox
        vec3f center_delta = -eye;
        this->v_skybox = glm::lookAt(vec3f(0.0f, 0.0, 0.0f), center_delta, vec3f(up));
    }
    if (this->res != res || this->camera != cam)
    {
        vec2f resf = vec2f(res);
        auto distance = static_cast<float>(cam.distance);

        if (!cam.settings.is_render_perspective)
        {
            auto sv = static_cast<float>(distance) * resf / 300.0f;
            this->p = glm::ortho(-sv.x / 2.0f, sv.x / 2.0f, -sv.y / 2.0f, sv.y / 2.0f, distance - sv.x / 2.0f,
                                 distance + sv.y / 2.0f);
        }
        else
        {
            this->p = glm::perspective(cam.settings.fov, resf.x / resf.y, 0.1f, distance * 2.0f);
        }
        this->p_inverse = glm::inverse(this->p);
        changed = true;

        // SkyBox
        this->p_skybox =
            glm::perspective(glm::half_pi<float>(), static_cast<float>(res.x) / static_cast<float>(res.y), 0.01f, 2.0f);
    }
    if (changed)
    {
        this->vp = this->p * this->v;
        this->vp_inverse = glm::inverse(this->vp);
        this->vp_skybox = this->p_skybox * this->v_skybox;
    }
    this->camera = cam;
    this->res = res;
}

vec3f Transform2D::apply(vec3d pos)
{
    return this->vp * vec4f(pos + this->delta_transform, 1.0f);
}
vec3d Transform2D::inverse(vec3f pos)
{
    return vec3d(this->vp_inverse * vec4f(pos, 1.0f)) - this->delta_transform;
}

vec3d Renderer::cordOnTargetToWorldCord(vec2f cord_on_target, const Camera &cam, double z, sf::RenderTarget &target)
{

    this->transform2D.recalculate(cam, target.getSize());

    vec2f screen_size = vec2f(vec2u(target.getSize()));
    vec2f gl_cord = 2.0f * cord_on_target / screen_size - 1.0f;
    vec3d world_cord = this->transform2D.inverse({gl_cord.x, -gl_cord.y, z});
    return world_cord;
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
        if (cam.settings.is_fixed_body_size)
        {
            radius = 1 * cam.distance * cam.settings.fixed_size / 12.0;
        }
        else if (cam.settings.is_scaled_body_size)
        {
            radius = property.size.x * cam.settings.body_scale;
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
    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);
    this->frameBuffer.resize(target.getSize());

    auto &shader = gl::getResourcesGL()->mainShader;
    auto &shader_blur = gl::getResourcesGL()->shader_blur;
    auto &shader_combine = gl::getResourcesGL()->shader_combine;
    auto &shader_basic = gl::getResourcesGL()->shader_basic;

    // Render bodies
    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    shader.setTransparency(transparency);
    shader.setColorExt(color_addon);
    render2D(env, cam, shader);

    //////// Blur
    // auto blur_res = viewport / 6u;
    // glViewport(0, 0, blur_res.x, blur_res.y);
    // this->frameBuffer_blur.resize(blur_res);

    //// Horizontal Blur
    // this->frameBuffer_blur.texture_1.clear(Color::Transparent);
    // this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_1, 0, 0, 0);
    // shader_blur.setTexture(this->frameBuffer.texture_2);
    // shader_blur.setIsVertical(false);
    // shader_blur.use();
    // gl::getResourcesGL()->quad.render();

    //// Vertical Blur
    // this->frameBuffer_blur.texture_2.clear(Color::Transparent);
    // this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_2, 0, 0, 0);
    // shader_blur.setTexture(this->frameBuffer_blur.texture_1);
    // shader_blur.setIsVertical(false);
    // shader_blur.use();
    // gl::getResourcesGL()->quad.render();

    // this->frameBuffer_blur.texture_3.clear(Color::Transparent);
    // this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_3, 0, 0, 0);
    // shader_blur.setTexture(this->frameBuffer_blur.texture_2);
    // shader_blur.setIsVertical(true);
    // shader_blur.use();
    // gl::getResourcesGL()->quad.render();

    //// Combine Blur
    // glViewport(0, 0, viewport.x, viewport.y);

    // this->frameBuffer.texture_2.clear(Color::Transparent);
    // this->frameBuffer.activate(gl::FrameBuffer::Slot_2, 0, 0, 0);
    // shader_combine.setTexture1(this->frameBuffer.texture_1);
    // shader_combine.setTexture2(this->frameBuffer_blur.texture_3);
    // shader_combine.use();
    // gl::getResourcesGL()->quad.render();

    this->target->setActive(true);
    shader_basic.setTexture(this->frameBuffer.texture_1);
    shader_basic.use();
    gl::getResourcesGL()->quad.render();
}

void Renderer::renderSkyBox(gl::Texture &skybox, const Camera &cam, float transparency)
{
    assert(this->target);
    auto &target = *this->target;
    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);
    this->frameBuffer.resize(target.getSize());
    this->transform2D.recalculate(cam, viewport);

    auto &shader = gl::getResourcesGL()->mainShader;

    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    shader.setBrightness(0.2f);
    shader.setColor(Color::Transparent);
    shader.setTransparency(transparency);
    shader.setColorExt(Color::Transparent);
    shader.setMatrixM(mat4f(1.0f));
    shader.setMatrixP(this->transform2D.vp_skybox);
    shader.setTexture(skybox);
    shader.use();
    gl::getResourcesGL()->sphere.render();
}

void Renderer::renderGrid(double scale, const Camera &cam, float transparency, Color color)
{
    assert(this->target);
    auto &target = *this->target;
    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);
    this->frameBuffer.resize(target.getSize());

    auto &shader = gl::getResourcesGL()->mainShader;
    double exponant_1 = std::floor(std::log10(cam.distance * 0.6));
    double exponant_2 = std::floor(std::log10(cam.distance) + 1);

    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    this->renderGrid2D(exponant_1, cam, shader, Color(0.5f, 0.5f, 0.5f), transparency);
    this->renderGrid2D(exponant_2, cam, shader, Color(1.0f, 1.0f, 1.0f), transparency);

    // Debug
    phys::showDebugF("Exponent: {}", exponant_1);
}

mat4f getModelTransform(const vec4d pos_world, const vec4d size_world, const Transform2D transform)
{

    auto pos_scene = vec4d(transform.delta_transform, 0.0) + pos_world;
    mat4f model = mat4f(1.0f);
    model = glm::translate(model, vec3f(pos_scene));
    model = glm::scale(model, vec3f(size_world));
    return model;
}

void Renderer::renderGrid2D(double exponant, const Camera &cam, gl::ShaderMain &shader, Color color, float transparency)
{
    assert(this->target);
    auto &target = *this->target;

    this->transform2D.recalculate(cam, target.getSize());

    // Grid Rendering
    auto &vertexArrayGrid = gl::getResourcesGL()->grid;
    const double scale = std::pow(10, exponant);

    const auto amountGrid = gl::getResourcesGL()->gridAmount;
    const auto center_world = vec4d(vec2d(glm::round(cam.center / scale) * scale), cam.center.z, 1.0f);
    const auto size_world = vec4d{scale * amountGrid / 2.0, scale * amountGrid / 2.0, scale * amountGrid / 2.0, 0.0};

    const auto model = getModelTransform(center_world, size_world, transform2D);
    shader.setMatrixM(model);
    shader.setMatrixP(transform2D.vp);
    shader.setColor(color);
    shader.setColorExt(color);
    shader.setTransparency(transparency);
    shader.setBrightness(0.0f);
    shader.setFancy(false);

    shader.use();
    vertexArrayGrid.renderLines();
}

void Renderer::render2D(const Environment &env, const Camera &cam, gl::ShaderMain &shader)
{
    assert(env.properties.size() >= env.bodies.size());
    assert(this->target);
    auto &target = *this->target;

    // Pre calculate View Perspective for double calculations
    this->transform2D.recalculate(cam, target.getSize());

    // Fancy shadow rendering
    if (cam.settings.is_render_fancy)
    {
        auto bodies_zip = std::views::zip(env.bodies, env.properties);
        auto &&body_pair = std::ranges::find_if(bodies_zip,
                                                [](auto &&zipped)
                                                {
                                                    auto &&[body, prop] = zipped;
                                                    if (prop.name == "Sun")
                                                        return true;
                                                    return false;
                                                });
        if (body_pair != bodies_zip.end())
        {
            auto &&[body, prop] = *body_pair;
            // auto sun_pos = vp_world * vec4d(body.pos, 1.0);
            // shader.setSunPosition(vec3f(sun_pos));
        }
    }
    shader.setFancy(cam.settings.is_render_fancy);

    // View Perspective for float calculation in shader
    shader.setMatrixP(this->transform2D.vp);
    shader.use();

    // Render Bodies
    auto &vertexArray = gl::getResourcesGL()->sphere;
    for (auto [index, body] : std::views::enumerate(env.bodies))
    {
        auto &property = env.properties[index];
        // Model Configuring
        auto pos_world = vec4d(body.pos, 1.0f);
        auto size_world = vec4d(property.size, 0.0f);
        if (cam.settings.is_fixed_body_size)
        {
            size_world = vec4d(1.0, 1.0, 1.0, 0.0) * cam.distance * cam.settings.fixed_size / 12.0;
        }
        else if (cam.settings.is_scaled_body_size)
        {
            size_world = vec4d(property.size, 0.0f) * cam.settings.body_scale;
        }

        auto model = getModelTransform(pos_world, size_world, transform2D);
        shader.setMatrixM(model);
        shader.setBrightness(property.brightness);

        // Color + Texture
        auto color = env.properties[index].color;
        if (env.properties[index].texture && cam.settings.is_render_textures)
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
