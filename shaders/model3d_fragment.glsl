#version 300 es

// 3D 模型片段 Shader
precision mediump float;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;

out vec4 fragColor;

uniform sampler2D uTexture;
uniform bool uHasTexture;
uniform vec3 uLightDir;
uniform vec3 uColor;

void main() {
    // 环境光
    vec3 ambient = 0.2 * uColor;
    
    // 漫反射
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uColor;
    
    vec3 result = ambient + diffuse;
    
    if (uHasTexture) {
        vec3 texColor = texture(uTexture, vTexCoord).rgb;
        result *= texColor;
    }
    
    fragColor = vec4(result, 1.0);
}
