#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

layout (location = 0) uniform sampler2D u_Tex_1;
layout (location = 1) uniform sampler2D u_Tex_2;

void main() {
  vec4 color_1 =texture(u_Tex_1, TexCoords);
  vec4 color_2 =texture(u_Tex_2, TexCoords);
  //vec4 color = color_1 * (1-color_2.a) + color_2 * (color_2); 
  vec4 color = color_1 + color_2;

  FragColor = color;
}
