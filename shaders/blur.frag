#version 330 core

in  vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uImage;
uniform bool      uHorizontal;

// 9-tap separable Gaussian weights (sigma ~= 2)
const float weight[5] = float[](
    0.2270270270,
    0.1945945946,
    0.1216216216,
    0.0540540541,
    0.0162162162
);

void main() {
    vec2 stride = (1.0 / textureSize(uImage, 0)) * 2.0;
    vec2 dir    = uHorizontal ? vec2(stride.x, 0.0) : vec2(0.0, stride.y);

    vec3 result = texture(uImage, vTexCoord).rgb * weight[0];
    for (int i = 1; i < 5; ++i) {
        result += texture(uImage, vTexCoord + dir * float(i)).rgb * weight[i];
        result += texture(uImage, vTexCoord - dir * float(i)).rgb * weight[i];
    }

    FragColor = vec4(result, 1.0);
}
