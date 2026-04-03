// ============================================================================
// PostProcessor - Grayscale Fragment Shader
// ============================================================================
// 用途：灰度效果
// 输入：
//   - uTexture: 输入纹理
// 说明：使用亮度公式将彩色转换为灰度
// ============================================================================

precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D uTexture;

void main() {
    vec4 color = texture2D(uTexture, vTexCoord);
    
    // 使用标准亮度公式计算灰度值
    // NTSC 标准：0.299R + 0.587G + 0.114B
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    
    // 也可以使用平均值法（更简单）
    // float gray = (color.r + color.g + color.b) / 3.0;
    
    gl_FragColor = vec4(gray, gray, gray, color.a);
}
