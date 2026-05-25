#version 330 core

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uModel;

layout(location = 0) in vec3 aPos;

void main() {
    vec4 clip = uProjection * uView * uModel * vec4(aPos, 1.0);
    gl_Position = clip;
    gl_PointSize = clamp(14.0 / sqrt(clip.w) + 2.0, 4.0, 12.0);
}
