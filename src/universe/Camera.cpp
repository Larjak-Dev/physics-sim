#include "Camera.hpp"
#include <glm/ext/matrix_transform.hpp>

using namespace phys;

mat4d Camera::getRotationMatrix() const
{

    auto rotation_matrix = mat4d(1.0);
    rotation_matrix = glm::rotate(rotation_matrix, this->z_angle, vec3d(0.0, 0.0, 1.0));
    rotation_matrix = glm::rotate(rotation_matrix, this->x_angle, vec3d(1.0, 0.0, 0.0));
    return rotation_matrix;
}

vec3d Camera::getEye() const
{
    auto rotation_matrix = this->getRotationMatrix();
    auto eye = vec4d(this->center, 1.0) + rotation_matrix * vec4d(0.0, 0.0, this->distance, 0.0);
    return vec3d(eye);
}

vec3d Camera::getCrossX() const
{

    auto rotation_matrix = this->getRotationMatrix();
    auto x = rotation_matrix * vec4d(1.0, 0.0, 0.0, 1.0);
    return x;
}
vec3d Camera::getCrossY() const
{
    auto rotation_matrix = this->getRotationMatrix();
    auto y = rotation_matrix * vec4d(0.0, 1.0, 0.0, 1.0);
    return y;
}
vec3d Camera::getCrossZ() const
{
    auto rotation_matrix = this->getRotationMatrix();
    auto z = rotation_matrix * vec4d(0.0, 0.0, 1.0, 1.0);
    return z;
}
