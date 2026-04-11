#version 330 core
#extension GL_ARB_separate_shader_objects : enable

out vec4 FragColor;

layout(location = 0) in vec3 aPos;

const vec3 starColor = vec3(1.0, 1.0, 1.0);

// gaussian falloff constant
const float sharpness = 5.0;

void main()
{
    vec2 coord = 2.0 * gl_PointCoord - 1.0; // center at 0
    float radSq = dot(coord, coord);
    if (radSq > 1.0) discard;

    float falloff = exp(-radSq * sharpness);
    FragColor = vec4(starColor, falloff);
}
