#version 460 core

in vec4 vertexColor;
in vec2 texCord;
in float brightness;

layout (location = 14) uniform float u_transparency;
layout (location = 30) uniform sampler2D ourTexture;
layout (location = 31) uniform sampler2D ourTextureDark;
layout (location = 32) uniform bool hasDarkTexture;
layout (location = 33) uniform sampler2D ourTextureAtmosphere;
layout (location = 34) uniform bool hasAtmosphereTexture;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

void main()
{
    vec4 baseTex = texture(ourTexture, texCord);
    vec4 color_bright = vec4(mix(baseTex.rgb, vertexColor.rgb, vertexColor.a), mix(baseTex.a, 1.0, vertexColor.a));
    color_bright.a *= u_transparency;

    if(hasAtmosphereTexture) {
        vec4 color_atmosphere = vec4(mix(texture(ourTextureAtmosphere, texCord).rgb, vertexColor.rgb, vertexColor.a), 1.0);
        float transition_factor = (color_atmosphere.r + color_atmosphere.g + color_atmosphere.b)/3.0;
        color_bright = vec4(mix(color_bright.rgb, vec3(1.0), transition_factor), color_bright.a);
    }
    
    if (hasDarkTexture) {
        vec4 color_dark = vec4(mix(texture(ourTextureDark, texCord).rgb, vertexColor.rgb, vertexColor.a), 1.0);
        float transition_factor = (1.0+min(brightness, 0.0));
        FragColor = vec4(mix(color_dark.rgb, color_bright.rgb, transition_factor), color_bright.a);
    } else {
        FragColor = vec4(color_bright.rgb * (1.0+min(brightness, 0.0)), color_bright.a);
    }
    
    BrightColor = FragColor * max(brightness, 0.0);
}
