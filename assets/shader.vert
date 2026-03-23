#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCord;
layout (location = 2) in vec3 aNormal;

layout (location = 0) uniform mat4 u_VP;
layout (location = 4) uniform mat4 u_V;
layout (location = 8) uniform mat4 u_M;
layout (location = 12) uniform vec4 u_color;
layout (location = 13) uniform vec4 u_color_ext;
layout (location = 14) uniform float u_transparency;
layout (location = 15) uniform float u_brightness;
layout (location = 16) uniform vec3 u_body_center;
layout (location = 17) uniform vec3 u_sun_pos;
layout (location = 18) uniform bool u_fancy;
layout (location = 19) uniform mat4 u_normal_matrix;

out vec4 vertexColor;
out vec2 texCord;
out float brightness;

float easeOutQuint(float x) {
    return 1.0 - pow(1.0-x,5.0);
}

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

        float strength = easeOutQuint(max(similarity, 0.0)); //0.0-1.0

        brightness = ((1.0 + u_brightness) * strength) - 1.0;
    } else {
        brightness = u_brightness;
    }

}
