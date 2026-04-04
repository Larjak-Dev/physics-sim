#pragma once
#include "../core/Units.hpp"
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
    virtual ~Texture();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    void bindUnit(uint32_t unit) const;

    void resize(vec2u size);
    void setFilter(uint32_t min_filter, uint32_t mag_filter);
    void loadFromImage(std::string path);
    void createColor(Color color);
    void clear(Color color);

    uint32_t getID();

  protected:
    uint32_t texture_id{0};
};

class FrameBuffer
{
  public:
    FrameBuffer() = default;
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer &) = delete;
    FrameBuffer &operator=(const FrameBuffer &) = delete;

    void resize(vec2u size);
    void activate(uint32_t attachment_1 = 36064, uint32_t attachment_2 = 36065, uint32_t attachment_3 = 36066,
                  uint32_t attachment_4 = 36067) const;

    // Bind a specific attachment to a texture unit
    void bindTexture(uint32_t unit) const;
    void bindFrameBuffer(uint32_t index) const;
    void activate_zdepth();
    void deactive_zdepth();

    uint32_t fbo_id{0};
    uint32_t rbo_id{0};

    Texture texture_1;
    Texture texture_2;
    Texture texture_3;
    Texture texture_4;

    inline static const uint32_t Slot_1 = 36064;
    inline static const uint32_t Slot_2 = 36065;
    inline static const uint32_t Slot_3 = 36066;
    inline static const uint32_t Slot_4 = 36067;

  private:
    vec2u size{0, 0};
};

class Shader
{
  public:
    Shader(const std::string &vert, const std::string &frag);
    ~Shader();
    uint32_t getShaderHandle() const;

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    void use() const;

  private:
    uint32_t shader_program{0};
};

class ShaderMain : public Shader
{
  public:
    ShaderMain();

    void setMatrixVP(mat4f view_projection);
    void setMatrixV(mat4f view);
    void setMatrixM(mat4f model);
    void setColor(Color color);
    void setColorExt(Color color);
    void setTexture(const Texture &texture);
    void setTextureDarkSide(const Texture &texture);
    void setHasDarkSide(bool hasDarkSide);
    void setTextureAtmosphere(const Texture &texture);
    void setHasAtmosphere(bool hasAtmosphere);
    void setTransparency(float transparency);
    void setBrightness(float brightness);
    void setBodyPosition(vec3f pos);
    void setSunPosition(vec3f pos);
    void setFancy(bool isFancy);
    void setMatrixNormal(mat4f normal_matrix);
};
class ShaderBasic : public Shader
{
  public:
    ShaderBasic();

    void setTexture(Texture &texture);
};

class ShaderBlur : public Shader
{
  public:
    ShaderBlur();

    void setTexture(Texture &texture);
    void setIsVertical(bool isVertical);
};
class ShaderCombine : public Shader
{
  public:
    ShaderCombine();

    void setTexture1(Texture &texture);
    void setTexture2(Texture &texture);
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
    void bufferQuad();

    void render();
    void renderLines();
    void renderPoints();

  private:
    uint32_t VAO{0}, VBO{0}, EBO{0}, VBO_TEX{0}, VBO_NORMAL{0};
    uint32_t indices{0};
};

} // namespace phys::gl
