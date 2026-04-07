#version 330 core

uniform mat4 uView;
uniform mat4 uProjection;

layout(location = 0) in vec3 aPos;

void main() {
    gl_Position = uProjection * uView * vec4(aPos, 1.0);
    gl_PointSize = 3.0;
}
