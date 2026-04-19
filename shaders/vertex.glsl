#version 330 core

const float minSize = 2.0;
const float baseSize = 10.0;
const float maxSize = 1000.0;

const float uNear = 0.01;
const float uFar = 1000.0;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uModel;

layout(location = 0) in vec3 aPos;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);

    float size = max(baseSize / gl_Position.w, minSize);

    gl_PointSize = size;
}
