#version 460 core

layout (location = 0) in vec3 aPos;

layout (location = 0) uniform mat4 u_VP;
layout (location = 1) uniform mat4 u_M;

out vec4 vertexColor;

void main()
{
    gl_Position = u_VP * u_M * vec4(aPos, 1.0);
    
    vertexColor = vec4(0.5, 0.0, 0.5, 1.0); // Dark Purple
}
