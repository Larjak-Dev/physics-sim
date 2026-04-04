#include "GladWrap.hpp"
#include "core/Units.hpp"
#include "core/tools/Error.hpp"
#include "glm/fwd.hpp"
#include <SFML/Graphics/Image.hpp>
#include <fstream>
#include <glad.h>
#include <ranges>
#include <sstream>

#define PAR_SHAPES_IMPLEMENTATION
#include <glm/gtc/type_ptr.hpp>
#include <par_shapes.h>

using namespace phys::gl;

Texture::Texture()
{
}
Texture::~Texture()
{
    if (texture_id)
        glDeleteTextures(1, &texture_id);
}

void Texture::resize(vec2u size)
{
    glDeleteTextures(1, &texture_id);

    glCreateTextures(GL_TEXTURE_2D, 1, &this->texture_id);
    glTextureStorage2D(this->texture_id, 1, GL_RGBA8, size.x, size.y);
    glTextureParameteri(this->texture_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(this->texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(this->texture_id, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTextureParameteri(this->texture_id, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}

void Texture::setFilter(uint32_t min_filter, uint32_t mag_filter)
{
    if (this->texture_id)
    {
        glTextureParameteri(this->texture_id, GL_TEXTURE_MIN_FILTER, min_filter);
        glTextureParameteri(this->texture_id, GL_TEXTURE_MAG_FILTER, mag_filter);
    }
}

void Texture::bindUnit(GLuint unit) const
{
    assert(glIsTexture(this->texture_id));
    glBindTextureUnit(unit, this->texture_id);
}

void Texture::loadFromImage(std::string path)
{
    sf::Image image(path);
    if (auto data = image.getPixelsPtr())
    {
        auto width = image.getSize().x;
        auto height = image.getSize().y;
        resize({width, height});
        glTextureSubImage2D(this->texture_id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(this->texture_id);
    }
    else
    {
        resize({40, 40});
        float clearColor[] = {0.5f, 0.0f, 0.5f, 1.0f};
        glClearTexImage(this->texture_id, 0, GL_RGBA, GL_FLOAT, clearColor);
        showMessage("Failed To Load Texture");
    }
}

void Texture::createColor(Color color)
{
    resize({40, 40});
    float clearColor[] = {color.r, color.g, color.b, color.a};
    glClearTexImage(this->texture_id, 0, GL_RGBA, GL_FLOAT, clearColor);
}

void Texture::clear(Color color)
{
    if (!this->texture_id)
        return;
    float clearColor[4] = {color.r, color.g, color.b, color.a};
    glClearTexImage(this->texture_id, 0, GL_RGBA, GL_FLOAT, &clearColor);
}

uint32_t Texture::getID()
{
    return this->texture_id;
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::resize(vec2u size)
{
    if (this->size == size)
        return;
    this->size = size;
    glDeleteFramebuffers(1, &this->fbo_id);
    glCreateFramebuffers(1, &this->fbo_id);

    this->texture_1.resize(size);
    this->texture_2.resize(size);
    this->texture_3.resize(size);
    this->texture_4.resize(size);

    glNamedFramebufferTexture(this->fbo_id, GL_COLOR_ATTACHMENT0, this->texture_1.getID(), 0);
    glNamedFramebufferTexture(this->fbo_id, GL_COLOR_ATTACHMENT1, this->texture_2.getID(), 0);
    glNamedFramebufferTexture(this->fbo_id, GL_COLOR_ATTACHMENT2, this->texture_3.getID(), 0);
    glNamedFramebufferTexture(this->fbo_id, GL_COLOR_ATTACHMENT3, this->texture_4.getID(), 0);

    GLenum attatchments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glNamedFramebufferDrawBuffers(this->fbo_id, 4, attatchments);

    // RBO
    glDeleteRenderbuffers(1, &this->rbo_id);
    glCreateRenderbuffers(1, &this->rbo_id);

    glNamedRenderbufferStorage(this->rbo_id, GL_DEPTH_COMPONENT32F, size.x, size.y);
    glNamedFramebufferRenderbuffer(this->fbo_id, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rbo_id);

    if (glCheckNamedFramebufferStatus(this->fbo_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        phys::showMessage("Failed to generate framebuffer!");
    }
}

void FrameBuffer::activate(uint32_t attachment_1, uint32_t attachment_2, uint32_t attachment_3,
                           uint32_t attachment_4) const
{
    GLenum attatchments[4] = {attachment_1, attachment_2, attachment_3, attachment_4};
    glNamedFramebufferDrawBuffers(this->fbo_id, 4, attatchments);
    glBindFramebuffer(GL_FRAMEBUFFER, this->fbo_id);
}

void FrameBuffer::activate_zdepth()
{
    glEnable(GL_DEPTH_TEST);
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glDepthFunc(GL_GREATER);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void FrameBuffer::deactive_zdepth()
{
    glDisable(GL_DEPTH_TEST);
}

GLuint compileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        phys::showMessageF("Shader Compilation Error: {}", std::string(infoLog));
    }
    return shader;
}

GLuint createShaderProgram(const std::string &vertSrc, const std::string &fragSrc)
{
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex);
    glAttachShader(shader_program, frag);
    glLinkProgram(shader_program);

    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        phys::showMessageF("Failed to link shader! Info: {}", infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(frag);
    return shader_program;
}

std::string readFile(const std::string &path)
{
    std::ifstream fileStream(path);
    if (!fileStream.is_open())
    {
        phys::showMessageF("Couldnt open file {}", path);
        return "";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

Shader::Shader(const std::string &vertPath, const std::string &fragPath)
{
    glDeleteProgram(this->shader_program);

    auto vert = readFile(vertPath);
    auto frag = readFile(fragPath);
    this->shader_program = createShaderProgram(vert, frag);
}

Shader::~Shader()
{
    glDeleteProgram(this->shader_program);
}

GLuint Shader::getShaderHandle() const
{
    return this->shader_program;
}

void Shader::use() const
{
    glUseProgram(this->shader_program);
}

ShaderBasic::ShaderBasic() : Shader("assets/shader_basic.vert", "assets/shader_basic.frag")
{
    glProgramUniform1i(this->getShaderHandle(), 0, 0);
}

void ShaderBasic::setTexture(Texture &texture)
{
    texture.bindUnit(0);
}

ShaderMain::ShaderMain() : Shader("assets/shader.vert", "assets/shader.frag")
{
    glProgramUniform1i(this->getShaderHandle(), 30, 0);
    glProgramUniform1i(this->getShaderHandle(), 31, 1);
    glProgramUniform1i(this->getShaderHandle(), 33, 2);
}

void ShaderMain::setMatrixVP(mat4f view_projection)
{
    glProgramUniformMatrix4fv(this->getShaderHandle(), 0, 1, GL_FALSE, glm::value_ptr(view_projection));
}
void ShaderMain::setMatrixV(mat4f view)
{
    glProgramUniformMatrix4fv(this->getShaderHandle(), 4, 1, GL_FALSE, glm::value_ptr(view));
}
void ShaderMain::setMatrixM(mat4f model)
{

    glProgramUniformMatrix4fv(this->getShaderHandle(), 8, 1, GL_FALSE, glm::value_ptr(model));
}
void ShaderMain::setColor(Color color)
{
    glProgramUniform4f(this->getShaderHandle(), 12, color.r, color.g, color.b, color.a);
}
void ShaderMain::setColorExt(Color color)
{
    glProgramUniform4f(this->getShaderHandle(), 13, color.r, color.g, color.b, color.a);
}
void ShaderMain::setTexture(const Texture &texture)
{
    texture.bindUnit(0);
}

void ShaderMain::setTextureDarkSide(const Texture &texture)
{
    texture.bindUnit(1);
}

void ShaderMain::setHasDarkSide(bool hasDarkSide)
{
    glProgramUniform1i(this->getShaderHandle(), 32, hasDarkSide);
}

void ShaderMain::setTextureAtmosphere(const Texture &texture)
{
    texture.bindUnit(2);
}

void ShaderMain::setHasAtmosphere(bool hasAtmosphere)
{
    glProgramUniform1i(this->getShaderHandle(), 34, hasAtmosphere);
}

void ShaderMain::setTransparency(float transparency)
{
    glProgramUniform1f(this->getShaderHandle(), 14, transparency);
}
void ShaderMain::setBrightness(float brightness)
{
    glProgramUniform1f(this->getShaderHandle(), 15, brightness);
}

void ShaderMain::setBodyPosition(vec3f pos)
{
    glProgramUniform3f(this->getShaderHandle(), 16, pos.x, pos.y, pos.z);
}

void ShaderMain::setSunPosition(vec3f pos)
{
    glProgramUniform3f(this->getShaderHandle(), 17, pos.x, pos.y, pos.z);
}
void ShaderMain::setFancy(bool isFancy)
{
    glProgramUniform1i(this->getShaderHandle(), 18, isFancy);
}

void ShaderMain::setMatrixNormal(mat4f normal_matrix)
{
    glProgramUniformMatrix4fv(this->getShaderHandle(), 19, 1, GL_FALSE, glm::value_ptr(normal_matrix));
}

ShaderBlur::ShaderBlur() : Shader("assets/shader_basic.vert", "assets/shader_blur.frag")
{
    glProgramUniform1i(this->getShaderHandle(), 0, 0);
}

void ShaderBlur::setTexture(Texture &texture)
{
    texture.bindUnit(0);
}
void ShaderBlur::setIsVertical(bool isVertical)
{
    glProgramUniform1i(this->getShaderHandle(), 1, (int)isVertical);
}

ShaderCombine::ShaderCombine() : Shader("assets/shader_basic.vert", "assets/shader_combine.frag")
{
    glProgramUniform1i(this->getShaderHandle(), 0, 0);
    glProgramUniform1i(this->getShaderHandle(), 1, 1);
}

void ShaderCombine::setTexture1(Texture &texture)
{
    texture.bindUnit(0);
}
void ShaderCombine::setTexture2(Texture &texture)
{
    texture.bindUnit(1);
}

VertexArray::VertexArray()
{
}
VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->EBO);
}
void VertexArray::bufferMesh(par_shapes_mesh *mesh)
{
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->VBO_TEX);
    glDeleteBuffers(1, &this->VBO_NORMAL);
    glDeleteBuffers(1, &this->EBO);

    glCreateVertexArrays(1, &this->VAO);
    glCreateBuffers(1, &this->VBO);
    glCreateBuffers(1, &this->VBO_TEX);
    glCreateBuffers(1, &this->VBO_NORMAL);
    glCreateBuffers(1, &this->EBO);

    glNamedBufferStorage(this->VBO, mesh->npoints * 3 * sizeof(float), mesh->points, 0);
    glVertexArrayVertexBuffer(this->VAO, 0, this->VBO, 0, 3 * sizeof(float));

    glEnableVertexArrayAttrib(this->VAO, 0);
    glVertexArrayAttribFormat(this->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(this->VAO, 0, 0); // Explicitly link Location 0 to Binding 0

    if (mesh->tcoords != nullptr)
    {
        glNamedBufferStorage(this->VBO_TEX, mesh->npoints * 2 * sizeof(float), mesh->tcoords, 0);

        glVertexArrayVertexBuffer(this->VAO, 1, this->VBO_TEX, 0, 2 * sizeof(float));

        glEnableVertexArrayAttrib(this->VAO, 1);
        glVertexArrayAttribFormat(this->VAO, 1, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(this->VAO, 1, 1); // Explicitly link Location 1 to Binding 1
    }

    if (mesh->normals != nullptr)
    {

        glNamedBufferStorage(this->VBO_NORMAL, mesh->npoints * 3 * sizeof(float), mesh->normals, 0);

        glVertexArrayVertexBuffer(this->VAO, 2, this->VBO_NORMAL, 0, 3 * sizeof(float));

        glEnableVertexArrayAttrib(this->VAO, 2);
        glVertexArrayAttribFormat(this->VAO, 2, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(this->VAO, 2, 2); // Explicitly link Location 1 to Binding 1
    }

    glNamedBufferStorage(this->EBO, mesh->ntriangles * 3 * sizeof(uint16_t), mesh->triangles, 0);
    glVertexArrayElementBuffer(this->VAO, this->EBO);

    this->indices = mesh->ntriangles * 3;
}

void VertexArray::bufferLines(const std::vector<vec3f> points)
{
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->EBO);

    glCreateVertexArrays(1, &this->VAO);
    glCreateBuffers(1, &this->VBO);

    glNamedBufferStorage(this->VBO, points.size() * sizeof(phys::vec3f), points.data(), 0);

    glVertexArrayVertexBuffer(this->VAO, 0, this->VBO, 0, sizeof(phys::vec3f));

    glEnableVertexArrayAttrib(this->VAO, 0);
    glVertexArrayAttribFormat(this->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(this->VAO, 0, 0);

    this->indices = points.size();
}

void VertexArray::bufferLines(int x, int y, int z)
{
    const float difx = 2.0f / x;
    const float dify = 2.0f / y;
    const float difz = 2.0f / z;
    std::vector<vec3f> points;
    for (int i : std::views::iota(0, x))
    {
        points.push_back(vec3f(-1 + i * difx, -1.0f, 0.0f));
        points.push_back(vec3f(-1 + i * difx, 1.0f, 0.0f));
    }
    for (int i : std::views::iota(0, y))
    {
        points.push_back(vec3f(-1.0f, -1.0f + i * dify, 0.0f));
        points.push_back(vec3f(1.0f, -1.0f + i * dify, 0.0f));
    }
    bufferLines(points);
}

void VertexArray::bufferSphere(int detail)
{
    auto *mesh = par_shapes_create_parametric_sphere(detail, detail);

    // Rotate 90 degrees around X to bring poles from Z-axis to Y-axis (Up)
    float axis[] = {1.0f, 0.0f, 0.0f};
    par_shapes_rotate(mesh, PAR_PI, axis);

    // Fix UV mapping: par_shapes provides (U=phi, V=theta)
    // Standard mapping expects (U=theta/longitude, V=phi/latitude)
    for (int i = 0; i < mesh->npoints; i++)
    {
        float u_old = mesh->tcoords[i * 2];
        float v_old = mesh->tcoords[i * 2 + 1];
        mesh->tcoords[i * 2] = v_old;            // U is now around (longitude)
        mesh->tcoords[i * 2 + 1] = 1.0f - u_old; // V is now up/down (latitude), flip so 1.0 is North Pole
    }

    // Fix reversed texture on x axis
    for (int i = 0; i < mesh->npoints; i++)
    {
        mesh->tcoords[i * 2] = -mesh->tcoords[i * 2] + 1.0f;
    }

    bufferMesh(mesh);
    par_shapes_free_mesh(mesh);
}

void VertexArray::bufferQuad()
{
    const float positions[] = {
        1.0f,  1.0f,  0.0f, // 0: Top Right
        1.0f,  -1.0f, 0.0f, // 1: Bottom Right
        -1.0f, -1.0f, 0.0f, // 2: Bottom Left
        -1.0f, 1.0f,  0.0f  // 3: Top Left
    };

    const float texCoords[] = {
        1.0f, 1.0f, // 0: Top Right
        1.0f, 0.0f, // 1: Bottom Right
        0.0f, 0.0f, // 2: Bottom Left
        0.0f, 1.0f  // 3: Top Left
    };

    // Two triangles to form the square (Counter-Clockwise winding)
    const uint16_t indices[] = {
        0, 3, 1, // First Triangle (Top Right, Top Left, Bottom Right)
        1, 3, 2  // Second Triangle (Bottom Right, Top Left, Bottom Left)
    };

    // Clean up old buffers
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->VBO_TEX);
    glDeleteBuffers(1, &this->EBO);

    // Create new buffers
    glCreateVertexArrays(1, &this->VAO);
    glCreateBuffers(1, &this->VBO);
    glCreateBuffers(1, &this->VBO_TEX);
    glCreateBuffers(1, &this->EBO);

    // Position Buffer (Location 0, Binding 0) ---
    glNamedBufferStorage(this->VBO, sizeof(positions), positions, 0);
    glVertexArrayVertexBuffer(this->VAO, 0, this->VBO, 0, 3 * sizeof(float));

    glEnableVertexArrayAttrib(this->VAO, 0);
    glVertexArrayAttribFormat(this->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(this->VAO, 0, 0);

    // Texture Coordinate Buffer (Location 1, Binding 1) ---
    glNamedBufferStorage(this->VBO_TEX, sizeof(texCoords), texCoords, 0);
    glVertexArrayVertexBuffer(this->VAO, 1, this->VBO_TEX, 0, 2 * sizeof(float));

    glEnableVertexArrayAttrib(this->VAO, 1);
    glVertexArrayAttribFormat(this->VAO, 1, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(this->VAO, 1, 1);

    // Element Buffer ---
    glNamedBufferStorage(this->EBO, sizeof(indices), indices, 0);
    glVertexArrayElementBuffer(this->VAO, this->EBO);

    this->indices = 6;
}

void VertexArray::render()
{
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->indices, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}
void VertexArray::renderLines()
{
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_LINES, 0, this->indices);
    glBindVertexArray(0);
}
