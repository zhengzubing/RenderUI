// ============================================================================
// PostProcessor - Blur Fragment Shader (高斯模糊)
// ============================================================================
// 用途：高斯模糊效果
// 输入：
//   - uTexture: 输入纹理
//   - uBlurRadius: 模糊半径
//   - uResolution: 渲染目标分辨率
//   - uDirection: 模糊方向 (1.0, 0.0) 水平 或 (0.0, 1.0) 垂直
// 说明：使用两步模糊（水平 + 垂直）实现高效高斯模糊
// ============================================================================

precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uBlurRadius;
uniform vec2 uResolution;
uniform vec2 uDirection;

void main() {
    // 计算模糊权重的高斯分布
    float radius = uBlurRadius / uResolution.x; // 归一化半径
    
    if (radius < 0.001) {
        // 半径太小，直接返回原始颜色
        gl_FragColor = texture2D(uTexture, vTexCoord);
        return;
    }
    
    // 高斯模糊核权重（简化版 3-tap）
    vec4 colorSum = vec4(0.0);
    float weightSum = 0.0;
    
    // 中心像素
    colorSum += texture2D(uTexture, vTexCoord) * 0.4;
    weightSum += 0.4;
    
    // 两侧像素
    float weight = 0.3;
    colorSum += texture2D(uTexture, vTexCoord - vec2(radius, 0.0) * uDirection) * weight;
    colorSum += texture2D(uTexture, vTexCoord + vec2(radius, 0.0) * uDirection) * weight;
    weightSum += 2.0 * weight;
    
    gl_FragColor = colorSum / weightSum;
}
