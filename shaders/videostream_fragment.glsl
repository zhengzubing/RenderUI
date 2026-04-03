#version 300 es

// 视频流片段 Shader - YUV420 转 RGB
precision mediump float;

in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D uTextureY;
uniform sampler2D uTextureU;
uniform sampler2D uTextureV;

void main() {
    float y = texture(uTextureY, vTexCoord).r;
    float u = texture(uTextureU, vTexCoord).r - 0.5;
    float v = texture(uTextureV, vTexCoord).r - 0.5;
    
    // YUV420 到 RGB 转换矩阵
    float r = y + 1.402 * v;
    float g = y - 0.344 * u - 0.714 * v;
    float b = y + 1.772 * u;
    
    fragColor = vec4(r, g, b, 1.0);
}
