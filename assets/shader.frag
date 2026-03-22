#version 460 core

in vec4 vertexColor;
in vec2 texCord;
in float brightness;

uniform sampler2D ourTexture;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

void main()
{
    vec4 color = texture(ourTexture, texCord) * (1.0 - vertexColor.a) + vertexColor * vertexColor.a;
    FragColor = vec4(color.rgb * (1.0+min(brightness, 0.0)), color.a);
    BrightColor = FragColor * max(brightness,0.0);
}
