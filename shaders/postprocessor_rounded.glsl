// ============================================================================
// PostProcessor - Rounded Corners Fragment Shader
// ============================================================================
// 用途：圆角裁剪效果
// 输入：
//   - uTexture: 输入纹理
//   - uCornerRadius: 圆角半径 (0.0-0.5)
//   - uResolution: 渲染目标分辨率
// ============================================================================

precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uCornerRadius;
uniform vec2 uResolution;

void main() {
    vec2 uv = vTexCoord;
    vec2 center = uv - 0.5;
    float dist = length(center);
    
    // 圆角算法：计算到中心的距离
    if (dist > 0.5 - uCornerRadius) {
        // 使用 smoothstep 实现平滑过渡
        float alpha = 1.0 - smoothstep(0.5 - uCornerRadius, 
                                       0.5 - uCornerRadius + 0.01, 
                                       dist);
        gl_FragColor = texture2D(uTexture, uv) * vec4(1.0, 1.0, 1.0, alpha);
    } else {
        gl_FragColor = texture2D(uTexture, uv);
    }
}
