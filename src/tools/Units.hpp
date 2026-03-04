#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <glm/common.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>

struct ImVec2;

namespace phys
{
struct vec4d : public glm::vec<4, double>
{
    using glm::dvec4::dvec4;
};
struct vec3d : public glm::vec<3, double>
{
    using glm::dvec3::dvec3;
};

struct vec2d : public glm::vec<2, double>
{
    using glm::dvec2::dvec2;
};

struct vec4f : public glm::vec<4, float>
{
    using glm::fvec4::fvec4;
};

struct vec3f : public glm::vec<3, float>
{
    using glm::fvec3::fvec3;
};

struct vec2f : public glm::vec<2, float>
{
    using glm::fvec2::fvec2;
    vec2f(ImVec2 vec);
    inline operator sf::Vector2f() const
    {
        return {this->x, this->y};
    }
    operator ImVec2() const;
};

struct vec2u : public glm::vec<2, unsigned int>
{
    using glm::uvec2::uvec2;
    inline vec2u(const sf::Vector2u &other)
    {
        this->x = other.x;
        this->y = other.y;
    }
    inline operator sf::Vector2u() const
    {
        return {this->x, this->y};
    }
};

using mat4d = glm::mat<4, 4, double>;
using mat4f = glm::mat<4, 4, float>;

struct Color
{
    float r, g, b, a;
};

} // namespace phys
