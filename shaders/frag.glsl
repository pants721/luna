#version 330 core
#extension GL_ARB_separate_shader_objects : enable

out vec4 FragColor;

in float vPointSize;

layout(location = 0) in vec3 aPos;

const vec3 starColor = vec3(1.0, 1.0, 1.0);

// gaussian falloff constant
const float baseSharpness = 5.0;

void main()
{
    vec2 coord = 2.0 * gl_PointCoord - 1.0; // center at 0
    float radSq = dot(coord, coord);

    if (vPointSize > 2.0 && radSq > 1.0) discard;

    float sharpness = mix(1.0, baseSharpness, clamp(vPointSize / 5.0, 0.0, 1.0));
    float falloff = exp(-radSq * sharpness);

    float edgeFade = smoothstep(1.0, 0.8, radSq);

    FragColor = vec4(starColor, falloff * edgeFade);
}
