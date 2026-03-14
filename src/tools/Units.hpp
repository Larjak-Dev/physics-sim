#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <glm/common.hpp>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <glm/vec2.hpp>

struct ImVec2;

namespace phys
{
using vec4d = glm::dvec4;
using vec3d = glm::dvec3;
using vec2d = glm::dvec2;
using vec4f = glm::fvec4;
using vec3f = glm::fvec3;

struct vec2f : public glm::fvec2
{
    using glm::fvec2::fvec2;
    vec2f() = default;
    vec2f(const glm::fvec2 &v) : glm::fvec2(v)
    {
    }

    vec2f(ImVec2 vec);
    inline operator sf::Vector2f() const
    {
        return {this->x, this->y};
    }
    operator ImVec2() const;

    // Operators to maintain vec2f type
    inline vec2f &operator+=(const glm::fvec2 &v)
    {
        glm::fvec2::operator+=(v);
        return *this;
    }
    inline vec2f &operator-=(const glm::fvec2 &v)
    {
        glm::fvec2::operator-=(v);
        return *this;
    }
    inline vec2f &operator*=(const glm::fvec2 &v)
    {
        glm::fvec2::operator*=(v);
        return *this;
    }
    inline vec2f &operator/=(const glm::fvec2 &v)
    {
        glm::fvec2::operator/=(v);
        return *this;
    }
    inline vec2f &operator*=(float s)
    {
        glm::fvec2::operator*=(s);
        return *this;
    }
    inline vec2f &operator/=(float s)
    {
        glm::fvec2::operator/=(s);
        return *this;
    }
};

struct vec2u : public glm::uvec2
{
    using glm::uvec2::uvec2;
    vec2u() = default;
    vec2u(const glm::uvec2 &v) : glm::uvec2(v)
    {
    }

    inline vec2u(const sf::Vector2u &other)
    {
        this->x = other.x;
        this->y = other.y;
    }
    inline operator sf::Vector2u() const
    {
        return {this->x, this->y};
    }

    inline vec2u &operator+=(const glm::uvec2 &v)
    {
        glm::uvec2::operator+=(v);
        return *this;
    }
    inline vec2u &operator-=(const glm::uvec2 &v)
    {
        glm::uvec2::operator-=(v);
        return *this;
    }
};

using mat4d = glm::mat<4, 4, double>;
using mat4f = glm::mat<4, 4, float>;

struct Color
{
    float r{0.0f}, g{0.0f}, b{0.0f}, a{1.0f};
};

inline const double PI = 3.141592653589793;

} // namespace phys
