// ============================================================================
// PostProcessor - Color Adjust Fragment Shader
// ============================================================================
// 用途：颜色和对比度调整
// 输入：
//   - uTexture: 输入纹理
//   - uBrightness: 亮度 (0.0-2.0, 1.0 为原始值)
//   - uContrast: 对比度 (0.0-2.0, 1.0 为原始值)
// ============================================================================

precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uBrightness;
uniform float uContrast;

void main() {
    vec4 color = texture2D(uTexture, vTexCoord);
    
    // 亮度调整：直接加减
    color.rgb += (uBrightness - 1.0);
    
    // 对比度调整：以 0.5 为中心缩放
    color.rgb = (color.rgb - 0.5) * uContrast + 0.5;
    
    // 限制颜色范围到 [0, 1]
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    
    gl_FragColor = color;
}
