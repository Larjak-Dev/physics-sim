#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

layout (location = 0) uniform sampler2D ourTexture;
layout (location = 1) uniform bool isVertical;

float weight[11] = {
    0.132980, // Center
    0.125841, // 1st neighbor
    0.106701, // 2nd
    0.081129, // 3rd
    0.055218, // 4th
    0.033682, // 5th
    0.018416, // 6th
    0.009017, // 7th
    0.003955, // 8th
    0.001554, // 9th
    0.000547  // 10th neighbor
    };


void main() {
  vec2 texture_size = textureSize(ourTexture, 0);
  vec2 texture_offset = vec2(1.0,1.0)/texture_size;

  vec2 cord_offset = vec2(texture_offset.x * float(!isVertical), texture_offset.y * float(isVertical));
  
  vec3 color_sum = vec3(0.0,0.0,0.0);

  color_sum += texture(ourTexture, TexCoords).rgb * weight[0];
  for(int i = 1; i<11; i++) {
    color_sum += texture(ourTexture, TexCoords - cord_offset * i).rgb * weight[i];
    color_sum += texture(ourTexture, TexCoords + cord_offset * i).rgb * weight[i];
  }

  FragColor = vec4(color_sum,1.0);
  //FragColor = texture(ourTexture, TexCoords);
}

