#pragma once
#include "../tools/Units.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <string>

struct par_shapes_mesh_s;
typedef struct par_shapes_mesh_s par_shapes_mesh;

namespace phys::gl
{
class Texture
{
  public:
    Texture();
    Texture(vec2u size);
    virtual ~Texture();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    void bindUnit(uint32_t unit) const;

    void resize(vec2u size);
    void loadFromImage(std::string path);

  private:
    uint32_t texture_id{0};
};

class TextureRender : public Texture
{
  public:
    TextureRender(vec2u size);
    ~TextureRender();

    TextureRender(const TextureRender &) = delete;
    TextureRender &operator=(const TextureRender &) = delete;

    void resize(vec2u size);
    void bindUnit(uint32_t unit);
    void bindFrame();

  private:
    uint32_t frameBuffer{0};
};

class Shader
{
  public:
    Shader(const std::string &vert, const std::string &frag);
    ~Shader();
    uint32_t getShaderHandle() const;

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

  private:
    uint32_t shader_program{0};
};

class VertexArray
{
  public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray &) = delete;
    VertexArray &operator=(const VertexArray &) = delete;

    void bufferMesh(par_shapes_mesh *mesh);
    void bufferSphere(int detail);
    void bufferLines(const std::vector<vec3f> points);
    void bufferLines(int x, int y, int z);

    void render();
    void renderLines();

  private:
    uint32_t VAO{0}, VBO{0}, EBO{0};
    uint32_t indices{0};
};

} // namespace phys::gl
