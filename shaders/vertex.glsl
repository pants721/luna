#version 330 core

const float minSize = 5.0;
const float baseSize = 100.0;
const float maxSize = 200.0;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uModel;

layout(location = 0) in vec3 aPos;

void main() {
    vec4 eyePos = uView * uModel * vec4(aPos, 1.0);
    gl_Position = uProjection * eyePos;
    // gl_PointSize = baseSize;
    float size = max(baseSize / gl_Position.w, minSize);

    gl_PointSize = size;
    // gl_PointSize = max(gl_PointSize, 2.0);
}
