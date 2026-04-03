// ============================================================================
// PostProcessor - Vertex Shader
// ============================================================================
// 用途：后期处理效果的顶点着色器
// 功能：传递纹理坐标到片段着色器
// ============================================================================

attribute vec2 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
