#include "Renderer.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/System/Vector2.hpp"
#include "gl/ResourcesGl.hpp"
#include "glm/trigonometric.hpp"
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace phys;

TransformWorld phys::createTransformWorld2D(const Camera &cam, vec2u res)
{
    TransformWorld t;

    auto cam_rotation = mat4d(1.0);
    cam_rotation = glm::rotate(cam_rotation, cam.z_angle, vec3d(0.0, 0.0, 1.0));
    cam_rotation = glm::rotate(cam_rotation, cam.x_angle, vec3d(1.0, 0.0, 0.0));
    auto eye = vec4d(cam.center, 1.0) + cam_rotation * vec4d(0.0, 0.0, cam.distance, 1.0);
    t.v = glm::lookAt(vec3d(eye), cam.center, vec3d(0.0, 1.0, 0.0));

    // when 300 pixel z=10 = 10 meter
    vec2d resd = vec2d(res);
    auto sv = cam.distance * resd / 300.0;
    t.p = glm::ortho(-sv.x / 2.0, sv.x / 2.0, -sv.y / 2.0, sv.y / 2.0, cam.distance - sv.x / 2.0,
                     cam.distance + sv.y / 2.0);

    t.vp = t.p * t.v;
    return t;
}

TransformWorld phys::createTransformWorld3D(const Camera &cam, vec2u res)
{
    TransformWorld t;

    auto camRotation = mat4d(1.0);
    camRotation = glm::rotate(camRotation, cam.z_angle, vec3d(0.0, 0.0, 1.0));
    camRotation = glm::rotate(camRotation, cam.x_angle, vec3d(1.0, 0.0, 0.0));
    auto eye = vec4d(cam.center, 1.0) + camRotation * vec4d(0.0, 0.0, cam.distance, 1.0);
    t.v = glm::lookAt(vec3d(eye), cam.center, vec3d(0.0, 0.0, 1.0));

    // t.p = glm::perspective();
}

void Renderer::render(sf::RenderTarget &target, sf::Vector2u size, const Environment &env, const Camera &cam)
{

    auto &shader = gl::getResourcesGL()->mainShader;
    render2D(target, size, env, cam, shader);
}

void Renderer::render2D(sf::RenderTarget &target, sf::Vector2u size, const Environment &env, const Camera &cam,
                        const gl::Shader &shader)
{
    if (!target.setActive(true))
    {
        return;
    }
    auto viewport = target.getSize();
    glViewport(0, 0, viewport.x, viewport.y);

    glUseProgram(shader.getShaderHandle());

    glClearColor(0.5f, 0.5f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto &vertexArray = gl::getResourcesGL()->sphere;
    vertexArray.render();

    glUseProgram(0);

    target.popGLStates();
}
