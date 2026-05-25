#version 330 core

out vec4 FragColor;

void main() {
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float r = length(coord);
    if (r > 1.0) discard;

    // Solid dot that fills most of the disc, smooth edge to avoid hard circle
    float core       = exp(-r * r * 8.0) * 6.0;
    float edge_fade  = 1.0 - smoothstep(0.7, 1.0, r);
    float brightness = core * edge_fade;

    vec3 coreColor = vec3(1.00, 0.95, 0.85);
    vec3 haloColor = vec3(0.20, 0.35, 1.00);
    vec3 color = mix(haloColor, coreColor, exp(-r * 6.0));

    // alpha=1: pure additive blend, avoids brightness^2 squaring with GL_SRC_ALPHA
    FragColor = vec4(color * brightness, 1.0);
}
