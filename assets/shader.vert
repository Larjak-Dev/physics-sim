#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCord;
layout (location = 2) in vec3 aNormal;

layout (location = 0) uniform mat4 u_VP;
layout (location = 4) uniform mat4 u_M;
layout (location = 8) uniform vec4 u_color;
layout (location = 9) uniform vec4 u_color_ext;
layout (location = 10) uniform float u_transparency;
layout (location = 11) uniform float u_brightness;
layout (location = 12) uniform vec3 u_body_center;
layout (location = 13) uniform vec3 u_sun_pos;
layout (location = 14) uniform bool u_fancy;

out vec4 vertexColor;
out vec2 texCord;
out float brightness;

void main()
{
    vec4 vertex_pos = u_M * vec4(aPos, 1.0);
    gl_Position = u_VP * vertex_pos;
    
    vec3 color = (vec3(u_color_ext) * u_color_ext.a) + (vec3(u_color) * (1.0-u_color_ext.a));
    vertexColor = vec4(color, u_color.a * u_transparency);
    texCord = aTexCord;

    if(!u_fancy) {
        brightness = 0.0;
        return;
    }

    if(u_brightness < 0.8) {
        vec3 normal_sun = normalize(u_sun_pos - vec3(vertex_pos));
        float similarity = dot(aNormal, normal_sun);
        float strength = max(similarity, 0.0);

        brightness = ((1.0 + u_brightness) * strength) - 1.0;
    } else {
        brightness = u_brightness;
    }

}
