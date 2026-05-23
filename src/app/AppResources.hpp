#pragma once
#include "graphics/GladWrap.hpp"
#include "imgui.h"

namespace phys
{

class AppResources
{
  public:
    ImFont *font_regular;
    ImFont *font_small;
};

class GlResources
{
  public:
    static constexpr int grid_amount = 200;
    gl::ShaderMain mainShader{};
    gl::ShaderBasic shader_basic{};
    gl::ShaderBlur shader_blur{};
    gl::ShaderCombine shader_combine{};
    gl::ShaderField shader_field{};

    gl::VertexArray sphere{};
    gl::VertexArray grid{};
    gl::VertexArray quad{};

    gl::Texture default_tex;

    gl::Texture sun;
    gl::Texture mercury;
    gl::Texture venus;
    gl::Texture mars;
    gl::Texture saturn;
    gl::Texture neptune;
    gl::Texture uranus;
    gl::Texture jupiter;

    gl::Texture stars;
    gl::Texture moon;
    gl::Texture earth_day;
    gl::Texture earth_night;
    gl::Texture earth_clouds;
    gl::Texture saturn_ring;

    GlResources();
};

class AppContext
{
  public:
    AppResources resources_app;
    GlResources resources_gl;
};

} // namespace phys
