#version 330 core
out vec4 FragColor;

void main()
{
    vec2 coord = 2.0 * gl_PointCoord - 1.0; // center at 0
    if (dot(coord, coord) > 1.0) discard;    // outside circle
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);    // white
}
