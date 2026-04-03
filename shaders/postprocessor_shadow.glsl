// ============================================================================
// PostProcessor - Shadow Fragment Shader
// ============================================================================
// 用途：阴影效果
// 输入：
//   - uTexture: 输入纹理
//   - uShadowOffset: 阴影偏移量
//   - uShadowBlur: 阴影模糊强度
// ============================================================================

precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec2 uShadowOffset;
uniform float uShadowBlur;

void main() {
    // 获取原始颜色
    vec4 color = texture2D(uTexture, vTexCoord);
    
    // 获取阴影颜色（偏移的纹理采样）
    vec4 shadowColor = texture2D(uTexture, vTexCoord - uShadowOffset);
    
    // 暗化阴影区域
    shadowColor.rgb *= 0.3;
    shadowColor.a *= uShadowBlur;
    
    // 混合原始颜色和阴影
    gl_FragColor = color + shadowColor;
}
