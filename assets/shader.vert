#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCord;

layout (location = 0) uniform mat4 u_VP;
layout (location = 4) uniform mat4 u_M;
layout (location = 8) uniform vec4 u_color;
layout (location = 9) uniform vec4 u_color_ext;
layout (location = 10) uniform float u_transparency;
layout (location = 11) uniform float u_brightness;

out vec4 vertexColor;
out vec2 texCord;
out float brightness;

void main()
{
    gl_Position = u_VP * u_M * vec4(aPos, 1.0);
    
    vec3 color = (vec3(u_color_ext) * u_color_ext.a) + (vec3(u_color) * (1.0-u_color_ext.a));
    vertexColor = vec4(color, u_color.a * u_transparency);
    texCord = aTexCord;
}
