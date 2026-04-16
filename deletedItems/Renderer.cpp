#include "Renderer.hpp"
#include "app/AppResources.hpp"
#include "core/Environment.hpp"
#include "core/tools/Debug.hpp"
#include "core/universe/Camera.hpp"
#include "core/universe/Universe.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Window.hpp>
#include <glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

using namespace phys;

Transform2D::Transform2D()
{
}

void Transform2D::recalculate(const Camera &cam, vec2u res)
{
    this->camera = cam;
    this->res = res;

    float aspect = static_cast<float>(res.x) / static_cast<float>(res.y);

    // Standard view
    this->v = glm::lookAt(vec3f(0.0f, 0.0f, cam.distance), vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f));
    this->v_inverse = glm::inverse(this->v);

    this->p = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10000.0f);
    // Reversed-Z for better precision at distance
    // this->p = glm::perspective(glm::radians(45.0f), aspect, 10000.0f, 0.1f);
    this->p_inverse = glm::inverse(this->p);

    this->vp = this->p * this->v;
    this->vp_inverse = glm::inverse(this->vp);

    // Skybox view (no translation)
    this->v_skybox = glm::mat4(glm::mat3(this->v));
    this->p_skybox = this->p;
    this->vp_skybox = this->p_skybox * this->v_skybox;

    this->delta_transform = -cam.center;
}

vec3f Transform2D::apply(vec3d v)
{
    auto v_scene = v + this->delta_transform;
    auto v_clip = this->vp * vec4f(v_scene, 1.0f);
    return vec3f(v_clip) / v_clip.w;
}

vec3d Transform2D::inverse(vec3f v)
{
    auto v_scene_4 = this->vp_inverse * vec4f(v, 1.0f);
    auto v_scene = vec3d(v_scene_4) / static_cast<double>(v_scene_4.w);
    return v_scene - this->delta_transform;
}

Renderer::Renderer(AppContext &context) : context(context)
{
}

Renderer::Renderer(const Renderer &other) : context(other.context)
{
    this->transform2D = other.transform2D;
    this->target = other.target;
}

Renderer &Renderer::operator=(const Renderer &other)
{
    this->transform2D = other.transform2D;
    this->target = other.target;
    return *this;
}

void Renderer::activate(sf::RenderTarget &target)
{
    assert(!this->target);
    this->target = &target;
    if (!target.setActive(true))
    {
        return;
    }
}

void Renderer::deactivate()
{
    assert(this->target);
    this->target->setActive(false);
    this->target = nullptr;
}

vec3d Renderer::cordOnTargetToWorldCord(vec2f cord_on_target, const Camera &cam, double z, sf::RenderTarget &target)
{
    this->transform2D.recalculate(cam, target.getSize());
    vec2f screen_size = vec2f(vec2u(target.getSize()));
    vec2f gl_cord = 2.0f * cord_on_target / screen_size - 1.0f;
    vec3d world_cord = this->transform2D.inverse({gl_cord.x, -gl_cord.y, z});
    return world_cord;
}

unsigned int Renderer::cordOnTargetToBodyInWorld(vec2f cord_on_target, const Camera &cam, const EnvironmentBase &env,
                                                 const std::vector<Property> &properties, sf::RenderTarget &target)
{
    auto ray_start = cordOnTargetToWorldCord(cord_on_target, cam, 1.0, target);
    auto ray_end = cordOnTargetToWorldCord(cord_on_target, cam, 0.5, target);
    auto ray_dir = ray_end - ray_start;
    auto ray_delta_norm = glm::normalize(ray_dir);

    double selected_distance = std::numeric_limits<double>::max();
    const Body *selected_body = nullptr;

    for (auto &&[body, prop] : std::views::zip(env.bodies, properties))
    {
        double radius = prop.size.x;
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

    float depthVal = 0.0f; // Far for Reversed-Z
    glClearNamedFramebufferfv(this->frameBuffer.fbo_id, GL_DEPTH, 0, &depthVal);

    if (this->target)
    {
        this->target->clear(sf::Color(static_cast<int>(background.r * 255.0f), static_cast<int>(background.g * 255.0f),
                                      static_cast<int>(background.b * 255.0f),
                                      static_cast<int>(background.a * 255.0f)));
    }
}

void Renderer::renderBodies(const EnvironmentBase &env, const std::vector<Property> &properties, const Camera &cam,
                            float transparency, Color color_addon)
{
    assert(this->target);
    auto &target = *this->target;
    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);
    this->frameBuffer.resize(target.getSize());

    auto &resources_gl = context.resources_gl;
    auto &shader = resources_gl.mainShader;
    auto &shader_blur = resources_gl.shader_blur;
    auto &shader_combine = resources_gl.shader_combine;
    auto &quad = resources_gl.quad;

    // Render bodies
    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    this->frameBuffer.activate_zdepth();
    shader.setTransparency(transparency);
    shader.setColorExt(color_addon);
    renderBodies2D(env, properties, cam, shader);
    this->frameBuffer.deactive_zdepth();

    //////// Blur
    auto blur_res = viewport / 6u;
    glViewport(0, 0, blur_res.x, blur_res.y);
    this->frameBuffer_blur.resize(blur_res);

    // Horizontal Blur
    this->frameBuffer_blur.texture_1.clear(Color::Transparent);
    this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_1, 0, 0, 0);
    shader_blur.setTexture(this->frameBuffer.texture_2);
    shader_blur.setIsVertical(false);
    shader_blur.use();
    quad.render();

    // Vertical Blur
    this->frameBuffer_blur.texture_2.clear(Color::Transparent);
    this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_2, 0, 0, 0);
    shader_blur.setTexture(this->frameBuffer_blur.texture_1);
    shader_blur.setIsVertical(false);
    shader_blur.use();
    quad.render();

    this->frameBuffer_blur.texture_3.clear(Color::Transparent);
    this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_3, 0, 0, 0);
    shader_blur.setTexture(this->frameBuffer_blur.texture_2);
    shader_blur.setIsVertical(true);
    shader_blur.use();
    quad.render();

    // Combine Blur
    glViewport(0, 0, viewport.x, viewport.y);

    this->frameBuffer.texture_2.clear(Color::Transparent);
    this->frameBuffer.activate(gl::FrameBuffer::Slot_2, 0, 0, 0);
    shader_combine.setTexture1(this->frameBuffer.texture_1);
    shader_combine.setTexture2(this->frameBuffer_blur.texture_3);
    shader_combine.use();
    quad.render();

    // Final Render to target
    this->target->setActive(true);
    glViewport(0, 0, viewport.x, viewport.y);
    this->context.resources_gl.shader_basic.setTexture(this->frameBuffer.texture_2);
    this->context.resources_gl.shader_basic.use();
    quad.render();
}

void Renderer::renderBodiesAmalgamated(const std::vector<std::shared_ptr<Universe>> &universes,
                                       const std::vector<std::pair<float, Color>> &properties, const Camera &cam)
{
    assert(this->target);
    auto &target = *this->target;
    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);
    this->frameBuffer.resize(target.getSize());

    auto &resources_gl = context.resources_gl;
    auto &shader = resources_gl.mainShader;
    auto &shader_blur = resources_gl.shader_blur;
    auto &shader_combine = resources_gl.shader_combine;
    auto &quad = resources_gl.quad;

    // Render bodies
    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    this->frameBuffer.activate_zdepth();

    // Disable Depth Mask for amalgamation so they don't occlude each other due to Z-fighting!
    glDepthMask(GL_FALSE);

    // Render each universe in the amalgamation
    for (auto &&[i, universe] : std::views::enumerate(universes))
    {
        if (!universe || !universe->env)
            continue;

        shader.setTransparency(properties[i].first);
        shader.setColorExt(properties[i].second);
        renderBodies2D(universe->env->getEnvironment_safe(), universe->properties, cam, shader);
    }

    glDepthMask(GL_TRUE);
    this->frameBuffer.deactive_zdepth();

    //////// Blur
    auto blur_res = viewport / 6u;
    glViewport(0, 0, blur_res.x, blur_res.y);
    this->frameBuffer_blur.resize(blur_res);

    // Horizontal Blur
    this->frameBuffer_blur.texture_1.clear(Color::Transparent);
    this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_1, 0, 0, 0);
    shader_blur.setTexture(this->frameBuffer.texture_2);
    shader_blur.setIsVertical(false);
    shader_blur.use();
    quad.render();

    // Vertical Blur
    this->frameBuffer_blur.texture_2.clear(Color::Transparent);
    this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_2, 0, 0, 0);
    shader_blur.setTexture(this->frameBuffer_blur.texture_1);
    shader_blur.setIsVertical(false);
    shader_blur.use();
    quad.render();

    this->frameBuffer_blur.texture_3.clear(Color::Transparent);
    this->frameBuffer_blur.activate(gl::FrameBuffer::Slot_3, 0, 0, 0);
    shader_blur.setTexture(this->frameBuffer_blur.texture_2);
    shader_blur.setIsVertical(true);
    shader_blur.use();
    quad.render();

    // Combine Blur
    glViewport(0, 0, viewport.x, viewport.y);

    this->frameBuffer.texture_2.clear(Color::Transparent);
    this->frameBuffer.activate(gl::FrameBuffer::Slot_2, 0, 0, 0);
    shader_combine.setTexture1(this->frameBuffer.texture_1);
    shader_combine.setTexture2(this->frameBuffer_blur.texture_3);
    shader_combine.use();
    quad.render();

    // Final Render to target
    this->target->setActive(true);
    glViewport(0, 0, viewport.x, viewport.y);
    this->context.resources_gl.shader_basic.setTexture(this->frameBuffer.texture_2);
    this->context.resources_gl.shader_basic.use();
    quad.render();
}

void Renderer::renderSkyBox(const gl::Texture &skybox, const Camera &cam, float transparency)
{
    assert(this->target);
    auto &target = *this->target;
    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);
    this->frameBuffer.resize(target.getSize());
    this->transform2D.recalculate(cam, viewport);

    auto &resources_gl = context.resources_gl;
    auto &shader = resources_gl.mainShader;
    auto &sphere = resources_gl.sphere;

    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    shader.setBrightness(0.2f);
    shader.setColor(Color::Transparent);
    shader.setTransparency(transparency);
    shader.setColorExt(Color::Transparent);
    shader.setMatrixM(mat4f(1.0f));
    shader.setMatrixVP(this->transform2D.vp_skybox);
    shader.setTexture(skybox);
    shader.setFancy(false);
    shader.use();
    sphere.render();
}

void Renderer::renderGrids(double scale, const Camera &cam, float transparency, double grid_y, Color color_small,
                           Color color_big)
{
    assert(this->target);
    auto &target = *this->target;
    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);
    this->frameBuffer.resize(target.getSize());

    auto &resources_gl = context.resources_gl;
    auto &shader = resources_gl.mainShader;

    double exponant_1 = std::floor(std::log10(cam.distance * 0.6) * scale);
    double exponant_2 = std::floor(std::log10(cam.distance) + 1 * scale);

    this->frameBuffer.activate(gl::FrameBuffer::Slot_1, gl::FrameBuffer::Slot_2, 0, 0);
    this->frameBuffer.activate_zdepth();
    this->renderGrid2D(exponant_1, cam, shader, grid_y, color_small, transparency);
    this->renderGrid2D(exponant_2, cam, shader, grid_y, color_big, transparency);
    this->frameBuffer.deactive_zdepth();

    // Debug
    phys::showDebugF("Exponent: {}", exponant_1);
}

struct ModelTransform
{
    mat4f model_transform;
    mat4f normal_transform;
};

ModelTransform getModelTransform(const vec4d pos_world, const vec4d size_world, float tilt, float rotation,
                                 const Transform2D transform)
{

    auto pos_scene = vec4d(transform.delta_transform, 0.0) + pos_world;
    mat4f model = mat4f(1.0f);
    model = glm::translate(model, vec3f(pos_scene));
    model = glm::scale(model, vec3f(size_world));
    model = glm::rotate(model, tilt, vec3f(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation, vec3f(0.0f, 0.0f, 1.0f));

    mat4f normal = mat4f(1.0f);
    normal = glm::rotate(normal, tilt, vec3f(0.0f, 1.0f, 0.0f));
    normal = glm::rotate(normal, rotation, vec3f(0.0f, 0.0f, 1.0f));
    return {model, normal};
}

void Renderer::renderGrid2D(double exponant, const Camera &cam, gl::ShaderMain &shader, double grid_z, Color color,
                            float transparency)
{
    assert(this->target);
    auto &target = *this->target;
    this->transform2D.recalculate(cam, target.getSize());

    auto &resources_gl = context.resources_gl;
    auto &vertexArrayGrid = resources_gl.grid;

    // Grid Rendering
    const auto amount_grid = resources_gl.grid_amount;
    const double scale_grid = std::pow(10, exponant);
    const auto center_grid = vec4d(vec2d(glm::round(cam.center / scale_grid) * scale_grid), grid_z, 1.0f);
    const auto size_grid =
        vec4d{scale_grid * amount_grid / 2.0, scale_grid * amount_grid / 2.0, scale_grid * amount_grid / 2.0, 0.0};

    const auto transform = getModelTransform(center_grid, size_grid, 0.0f, 0.0f, transform2D);
    shader.setMatrixM(transform.model_transform);
    shader.setMatrixNormal(transform.normal_transform);
    shader.setMatrixVP(transform2D.vp);
    shader.setColor(color);
    shader.setColorExt(color);
    shader.setTransparency(transparency);
    shader.setBrightness(0.0f);
    shader.setFancy(false);

    shader.use();
    vertexArrayGrid.renderLines();
}

void Renderer::renderBodies2D(const EnvironmentBase &env, const std::vector<Property> &properties, const Camera &cam,
                              gl::ShaderMain &shader)
{
    assert(properties.size() >= env.bodies.size());
    assert(this->target);
    auto &target = *this->target;
    this->transform2D.recalculate(cam, target.getSize());

    auto &resources_gl = context.resources_gl;
    auto &vertexArray = resources_gl.sphere;
    auto &default_tex = resources_gl.default_tex;

    // Fancy shadow rendering
    if (cam.settings.is_render_fancy)
    {
        auto bodies_zip = std::views::zip(env.bodies, properties);
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
            auto sun_pos = this->transform2D.delta_transform + body.pos;
            shader.setSunPosition(vec3f(sun_pos));
        }
    }

    shader.setFancy(cam.settings.is_render_fancy);
    shader.setMatrixVP(this->transform2D.vp);
    shader.use();

    // Render Bodies
    for (auto &&[body, property] : std::views::zip(env.bodies, properties))
    {
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

        auto rotation = property.rotation_start + property.rotation_velocity * env.passed_time;

        auto transform = getModelTransform(pos_world, size_world, property.tilt, rotation, transform2D);
        shader.setMatrixM(transform.model_transform);
        shader.setMatrixNormal(transform.normal_transform);
        shader.setBrightness(property.brightness);

        // Color + Texture
        auto color = property.color;
        if (property.texture && cam.settings.is_render_textures)
        {
            shader.setTexture(*property.texture);
            shader.setColor({0, 0, 0, 0});
        }
        else
        {
            shader.setTexture(default_tex);
            shader.setColor(color);
        }

        if (property.texture_dark && cam.settings.is_render_textures)
        {
            shader.setTextureDarkSide(*property.texture_dark);
            shader.setHasDarkSide(true);
        }
        else
        {
            shader.setHasDarkSide(false);
        }

        if (property.texture_atmosphere && cam.settings.is_render_textures)
        {
            shader.setTextureAtmosphere(*property.texture_atmosphere);
            shader.setHasAtmosphere(true);
        }
        else
        {
            shader.setHasAtmosphere(false);
        }

        vertexArray.render();
    }
}
