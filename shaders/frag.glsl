#version 330 core
#extension GL_ARB_separate_shader_objects : enable

out vec4 FragColor;

const vec3 starColor = vec3(1.0, 1.0, 1.0);

void main()
{
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float r = length(coord);

    if (r > 1.0) discard;

    // // core + glow
    // float core = exp(-r * 25.0);
    // float glow = exp(-r * 6.0);
    // float intensity = core + 0.5 * glow;
    //
    // // soft edge
    // float edgeFade = 1.0 - smoothstep(0.9, 1.0, r);
    //
    // float alpha = intensity * edgeFade;
    //
    // FragColor = vec4(starColor * alpha, alpha);

    FragColor = vec4(starColor, 1.0);
}
