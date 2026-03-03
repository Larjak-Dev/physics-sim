#version 460 core

// Input: Vertex position from the VBO (Location 0)
layout (location = 0) in vec3 aPos;

// Output: Pass a color to the fragment shader
out vec4 vertexColor;

void main()
{
    // High-level: Set the position of the vertex
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    
    // Assign a default color based on position for debugging
    vertexColor = vec4(0.5, 0.0, 0.5, 1.0); // Dark Purple
}
