#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

layout (location = 0) uniform sampler2D ourTexture;
layout (location = 1) uniform bool isVertical;

float weight[11] = {
    0.132980,
    0.125841, 
    0.106701,
    0.081129,
    0.055218,
    0.033682,
    0.018416,
    0.009017,
    0.003955,
    0.001554,
    0.000547  
    };


void main() {
  vec2 texture_size = textureSize(ourTexture, 0);
float aspect_ratio = texture_size.x / texture_size.y;

  vec2 cord_offset;
  if(isVertical) {
    cord_offset = vec2(0.0,1.0/texture_size.y);
  } else {
    cord_offset = vec2((1.0/texture_size.x) * aspect_ratio, 0.0);
  }
  
  vec3 color_sum = vec3(0.0,0.0,0.0);

  color_sum += texture(ourTexture, TexCoords).rgb * weight[0];
  for(int i = 1; i<11; i++) {
    color_sum += texture(ourTexture, TexCoords - cord_offset * i).rgb * weight[i];
    color_sum += texture(ourTexture, TexCoords + cord_offset * i).rgb * weight[i];
  }

  FragColor = vec4(color_sum,1.0);
}

