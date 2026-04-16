#version 460 core

layout (location = 9) uniform float min_g;
layout (location = 10) uniform float max_g;

layout (location = 11) uniform int body_count;
layout (location = 12) uniform float transparency;

in vec2 TexCoords;
in vec3 PosCoords;

out vec4 FragColor;

struct Body {
    vec3 pos;
    float mass;
    vec3 color;
};

layout(std430, binding = 0) buffer BodyBuffer {
    Body bodies[];
};

void main() {
    vec3 g_sum = vec3(0,0,0);
    vec3 weighted_color_sum = vec3(0,0,0);
    float total_g_mag = 0.0;

    float G = 6.674E-11;

    for(int i=0; i<body_count; i++) {
        Body body = bodies[i];
        vec3 r_vec = body.pos - PosCoords;
        float r = length(r_vec);

        if (r < 1.0) continue; 

        // Stable calculation: g = (G * m / r^2) * (r_vec / r)
        float g_mag = (G * body.mass) / (r * r);
        vec3 g_addition = g_mag * (r_vec / r);

        // Weight colors by their local gravitational strength
        weighted_color_sum += body.color * g_mag;
        total_g_mag += g_mag;

        g_sum += g_addition;
    }

    // Normalize the mixed color
    vec3 color_mix = (total_g_mag > 1e-25) ? (weighted_color_sum / total_g_mag) : vec3(1,1,1);

    float g = length(g_sum);

    // Logarithmic scale for wide dynamic range (Solar System)
    float log_g = log(max(g, 1e-20)) / log(10.0);
    float log_min = log(max(min_g, 1e-20)) / log(10.0);
    float log_max = log(max(max_g, 1e-20)) / log(10.0);

    float g_factor = clamp((log_g - log_min) / (log_max - log_min), 0.0, 1.0); 

    // Sharpening: Raising to a power (e.g., 2.0 or 3.0) makes it "less strong"
    g_factor = pow(g_factor, 2.0);

    FragColor = vec4(color_mix, g_factor * transparency);
}
