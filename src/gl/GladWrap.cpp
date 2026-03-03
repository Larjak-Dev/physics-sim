// FORWARD DECLARATION HANDLING
#define PAR_SHAPES_IMPLEMENTATION
#include <par_shapes.h>

#include "GladWrap.hpp"

#include <SFML/Graphics/Image.hpp>

#include <glad/glad.h>

#include <fstream>
#include <sstream>

#include "../tools/Error.hpp"

using namespace phys::gl;

Texture::Texture()
{
}
Texture::Texture(vec2u size)
{
    resize(size);
}
Texture::~Texture()
{
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
void Texture::bindUnit(GLuint unit) const
{
    assert(glIsTexture(this->texture_id));
    glBindTextureUnit(unit, this->texture_id);
}

void Texture::loadFromImage(std::string path)
{
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        resize({width, height});
        glTextureSubImage2D(this->texture_id, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(this->texture_id);
    }
    else
    {
        resize({40, 40});
        float clearColor[] = {0.5f, 0.0f, 0.5f, 1.0f};
        glClearTexImage(this->texture_id, 0, GL_RGBA, GL_FLOAT, clearColor);
        showMessage("Failed To Load Texture");
    }

    stbi_image_free(data);
}

TextureRender::TextureRender(vec2u size) : Texture()
{
    resize(size);
}

TextureRender::~TextureRender()
{
    Texture::~Texture();
}

void TextureRender::resize(vec2u size)
{
    Texture::resize(size);
}
void TextureRender::bindUnit(uint32_t unit)
{
}
void TextureRender::bindFrame()
{
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
    glDeleteBuffers(1, &this->EBO);

    glCreateVertexArrays(1, &this->VAO);

    glCreateBuffers(1, &this->VBO);
    glCreateBuffers(1, &this->EBO);

    glNamedBufferStorage(this->VBO, mesh->npoints * 3 * sizeof(float), mesh->points, 0);
    glNamedBufferStorage(this->EBO, mesh->ntriangles * 3 * sizeof(uint16_t), mesh->triangles, 0);

    glVertexArrayVertexBuffer(this->VAO, 0, this->VBO, 0, 3 * sizeof(float));
    glVertexArrayElementBuffer(this->VAO, this->EBO);

    glEnableVertexArrayAttrib(this->VAO, 0);
    glVertexArrayAttribFormat(this->VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    this->indices = mesh->ntriangles * 3;
}
void VertexArray::bufferSphere(int detail)
{
    auto *mesh = par_shapes_create_parametric_sphere(detail, detail);
    bufferMesh(mesh);
    par_shapes_free_mesh(mesh);
}

void VertexArray::render()
{
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->indices, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}
