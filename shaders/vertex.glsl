#version 330 core

const float minSize = 2.0;
const float baseSize = 50.0;
const float maxSize = 100.0;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uModel;

layout(location = 0) in vec3 aPos;

out float vPointSize;

void main() {
    vec4 eyePos = uView * uModel * vec4(aPos, 1.0);
    gl_Position = uProjection * eyePos;
    // gl_PointSize = baseSize;
    float size = max(baseSize / gl_Position.w, minSize);

    vPointSize = size;
    gl_PointSize = size;
}
