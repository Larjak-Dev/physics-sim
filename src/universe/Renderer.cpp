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
#include <cmath>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>

using namespace phys;

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
    if (this->res != res)
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
}

void Renderer::render(const Environment &env, const Camera &cam, float transparency, Color color_addon)
{

    auto &shader = gl::getResourcesGL()->mainShader;
    glProgramUniform1f(shader.getShaderHandle(), 10, transparency);
    glProgramUniform4f(shader.getShaderHandle(), 9, color_addon.r, color_addon.g, color_addon.b, color_addon.a);
    render2D(env, cam, shader);
}

void Renderer::renderGrid(double scale, const Camera &cam, float transparency, Color color)
{
    auto &shader = gl::getResourcesGL()->mainShader;
    double exponant_1 = std::floor(std::log10(cam.distance * 0.6));
    double exponant_2 = std::floor(std::log10(cam.distance) + 1);
    phys::showDebugF("Exponent: {}", exponant_1);
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

void Renderer::renderGrid2D(double exponant, const Camera &cam, const gl::Shader &shader, Color color,
                            float transparency)
{

    assert(this->target);
    auto &target = *this->target;

    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);

    glUseProgram(shader.getShaderHandle());

    mat4f vp_gl = mat4f(1.0f);
    glProgramUniformMatrix4fv(shader.getShaderHandle(), 0, 1, GL_FALSE, glm::value_ptr(vp_gl));

    glProgramUniform4f(shader.getShaderHandle(), 8, color.r, color.g, color.b, color.a);
    glProgramUniform4f(shader.getShaderHandle(), 9, color.r, color.g, color.b, color.a);
    glProgramUniform1f(shader.getShaderHandle(), 10, transparency);

    this->transform2D.recalculate(cam, target.getSize());
    auto vp_world = this->transform2D.vp;

    // Grid Rendering
    auto &vertexArrayGrid = gl::getResourcesGL()->grid;
    const double scale = std::pow(10, exponant);

    const auto amountGrid = gl::getResourcesGL()->gridAmount;
    const auto center_world = vec4d(glm::round(cam.center / scale) * scale, 1.0f);
    const auto size_world = vec4d{scale * amountGrid / 2.0, scale * amountGrid / 2.0, scale * amountGrid / 2.0, 0.0};

    auto model = getModelTransform(center_world, size_world, vp_world);
    glProgramUniformMatrix4fv(shader.getShaderHandle(), 4, 1, GL_FALSE, glm::value_ptr(model));

    vertexArrayGrid.renderLines();
}

void Renderer::render2D(const Environment &env, const Camera &cam, const gl::Shader &shader)
{
    assert(env.properties.size() >= env.bodies.size());

    assert(this->target);
    auto &target = *this->target;

    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);

    glUseProgram(shader.getShaderHandle());

    mat4f vp_gl = mat4f(1.0f);
    glProgramUniformMatrix4fv(shader.getShaderHandle(), 0, 1, GL_FALSE, glm::value_ptr(vp_gl));

    this->transform2D.recalculate(cam, target.getSize());
    auto vp_world = this->transform2D.vp;

    // Bodies
    auto &vertexArray = gl::getResourcesGL()->sphere;
    for (auto [index, body] : std::views::enumerate(env.bodies))
    {
        auto pos_world = vec4d(body.pos, 1.0f);
        auto size_world = vec4d(env.properties[index].size, 0.0f);

        auto model = getModelTransform(pos_world, size_world, vp_world);
        glProgramUniformMatrix4fv(shader.getShaderHandle(), 4, 1, GL_FALSE, glm::value_ptr(model));

        auto color = env.properties[index].color;
        glProgramUniform4f(shader.getShaderHandle(), 8, color.r, color.g, color.b, color.a);
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
