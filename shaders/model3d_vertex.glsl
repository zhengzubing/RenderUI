#version 300 es

// 3D 模型顶点 Shader
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uNormalMatrix;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vFragPos;

void main() {
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vFragPos = vec3(worldPos);
    vNormal = mat3(uNormalMatrix) * aNormal;
    vTexCoord = aTexCoord;
    
    gl_Position = uProjectionMatrix * uViewMatrix * worldPos;
}
