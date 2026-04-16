#version 460 core

layout (location = 0) uniform mat4 u_VP;
layout (location = 4) uniform mat4 u_V;
layout (location = 8) uniform mat4 u_M;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoords;
out vec3 PosCoords;

void main() {
  gl_Position = u_VP * u_M * vec4(aPos,1.0);
  TexCoords = aTexCoord;
  PosCoords = vec3(u_M * vec4(aPos,1.0));
}
