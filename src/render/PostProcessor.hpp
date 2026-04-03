#pragma once

#include <GLES2/gl2.h>
#include <memory>
#include <string>

namespace Component {

/**
 * @brief 后期处理器
 * 
 * 提供 OpenGL 后期处理效果：
 * - 圆角裁剪
 * - 阴影
 * - 模糊
 * - 颜色调整
 */
class PostProcessor {
public:
    /**
     * @brief 后处理效果类型
     */
    enum class Effect {
        None,           // 无效果
        RoundedCorners, // 圆角
        Shadow,         // 阴影
        Blur,           // 模糊
        Brightness,     // 亮度调整
        Contrast,       // 对比度调整
        Grayscale       // 灰度
    };
    
    /**
     * @brief 后处理参数
     */
    struct Params {
        float cornerRadius = 0.0f;    // 圆角半径
        float shadowBlur = 0.0f;      // 阴影模糊
        float shadowOffset = 0.0f;    // 阴影偏移
        float blurRadius = 0.0f;      // 模糊半径
        float brightness = 1.0f;      // 亮度 (0-2)
        float contrast = 1.0f;        // 对比度 (0-2)
        bool grayscale = false;       // 灰度
    };
    
    PostProcessor();
    ~PostProcessor();
    
    /**
     * @brief 初始化后期处理器
     * @param width 渲染目标宽度
     * @param height 渲染目标高度
     * @return 是否成功
     */
    bool init(int width, int height);
    
    /**
     * @brief 开始离屏渲染
     */
    void beginOffscreen();
    
    /**
     * @brief 结束离屏渲染并应用效果
     * @param effect 效果类型
     */
    void endOffscreen(Effect effect = Effect::None);
    
    /**
     * @brief 应用圆角裁剪
     * @param radius 圆角半径
     */
    void applyRoundedCorners(float radius);
    
    /**
     * @brief 应用阴影
     * @param blur 模糊程度
     * @param offset 偏移量
     */
    void applyShadow(float blur, float offset);
    
    /**
     * @brief 应用高斯模糊
     * @param radius 模糊半径
     */
    void applyBlur(float radius);
    
    /**
     * @brief 应用颜色调整
     * @param brightness 亮度
     * @param contrast 对比度
     */
    void applyColorAdjust(float brightness, float contrast);
    
    /**
     * @brief 应用灰度效果
     */
    void applyGrayscale();
    
    /**
     * @brief 设置参数
     */
    void setParams(const Params& params) { params_ = params; }
    
    /**
     * @brief 获取参数
     */
    const Params& getParams() const { return params_; }
    
    /**
     * @brief 获取 FBO ID
     */
    GLuint getFbo() const { return fbo_; }
    
    /**
     * @brief 获取纹理 ID
     */
    GLuint getTexture() const { return texture_; }
    
    /**
     * @brief 调整大小
     */
    void resize(int width, int height);
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    GLuint fbo_ = 0;
    GLuint texture_ = 0;
    GLuint rbo_ = 0;  // Renderbuffer
    
    GLuint shaderProgram_ = 0;
    int uCornerRadiusLoc_ = -1;
    int uShadowOffsetLoc_ = -1;
    int uBrightnessLoc_ = -1;
    int uContrastLoc_ = -1;
    
    int width_ = 0;
    int height_ = 0;
    
    Params params_;
    bool initialized_ = false;
    
    bool createShader(const char* vertexSource, const char* fragmentSource);
    void bindFramebuffer();
    void unbindFramebuffer();
};

} // namespace Component
