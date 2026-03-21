#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

layout (location = 0) uniform sampler2D u_Tex;

void main() {
  vec4 color =texture(u_Tex, TexCoords);
  FragColor = color;
}
