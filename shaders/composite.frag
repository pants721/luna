#version 330 core

in  vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform sampler2D uBloom;

const float BLOOM_STRENGTH = 0.06;

void main() {
    vec3 scene = texture(uScene, vTexCoord).rgb;
    vec3 bloom = texture(uBloom, vTexCoord).rgb;

    vec3 hdr = scene + bloom * BLOOM_STRENGTH;

    // Reinhard extended tone mapping
    vec3 mapped = hdr / (hdr + vec3(1.0));

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}
