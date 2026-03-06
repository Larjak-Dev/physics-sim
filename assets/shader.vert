#version 460 core

layout (location = 0) in vec3 aPos;

layout (location = 0) uniform mat4 u_VP;
layout (location = 4) uniform mat4 u_M;
layout (location = 8) uniform vec4 u_color;
layout (location = 9) uniform vec4 u_color_ext;
layout (location = 10) uniform float u_transparency;

out vec4 vertexColor;

void main()
{
    gl_Position = u_VP * u_M * vec4(aPos, 1.0);
    
    vec3 color = (vec3(u_color_ext) * u_color_ext.a) + (vec3(u_color) * (1.0-u_color_ext.a));
    vertexColor = vec4(color, u_color.a * u_transparency);
}
