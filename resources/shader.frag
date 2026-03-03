#version 460 core

// Input: The color passed from the vertex shader
in vec4 vertexColor;

// Output: The final pixel color
out vec4 FragColor;

void main()
{
    // Simply output the color we received
    FragColor = vertexColor;
}
