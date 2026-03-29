#include "PostProcessor.hpp"
#include "Logger.hpp"

namespace Component {

// 基础 Vertex Shader
static const char* sVertexShader = R"(
    attribute vec2 aPosition;
    attribute vec2 aTexCoord;
    varying vec2 vTexCoord;
    
    void main() {
        gl_Position = vec4(aPosition, 0.0, 1.0);
        vTexCoord = aTexCoord;
    }
)";

// 圆角 Fragment Shader
static const char* sFragmentRounded = R"(
    precision mediump float;
    varying vec2 vTexCoord;
    uniform sampler2D uTexture;
    uniform float uCornerRadius;
    uniform vec2 uResolution;
    
    void main() {
        vec2 uv = vTexCoord;
        vec2 center = uv - 0.5;
        float dist = length(center);
        
        // 简单的圆角算法
        if (dist > 0.5 - uCornerRadius) {
            float alpha = 1.0 - smoothstep(0.5 - uCornerRadius, 0.5 - uCornerRadius + 0.01, dist);
            gl_FragColor = texture2D(uTexture, uv) * vec4(1.0, 1.0, 1.0, alpha);
        } else {
            gl_FragColor = texture2D(uTexture, uv);
        }
    }
)";

// 阴影 Fragment Shader
static const char* sFragmentShadow = R"(
    precision mediump float;
    varying vec2 vTexCoord;
    uniform sampler2D uTexture;
    uniform vec2 uShadowOffset;
    uniform float uShadowBlur;
    
    void main() {
        vec4 color = texture2D(uTexture, vTexCoord);
        
        // 简化阴影：降低亮度并偏移
        vec4 shadowColor = texture2D(uTexture, vTexCoord - uShadowOffset);
        shadowColor.rgb *= 0.3;  // 暗化
        shadowColor.a *= uShadowBlur;
        
        gl_FragColor = color + shadowColor;
    }
)";

// 颜色调整 Fragment Shader
static const char* sFragmentColorAdjust = R"(
    precision mediump float;
    varying vec2 vTexCoord;
    uniform sampler2D uTexture;
    uniform float uBrightness;
    uniform float uContrast;
    
    void main() {
        vec4 color = texture2D(uTexture, vTexCoord);
        
        // 亮度调整
        color.rgb += (uBrightness - 1.0);
        
        // 对比度调整
        color.rgb = (color.rgb - 0.5) * uContrast + 0.5;
        
        gl_FragColor = color;
    }
)";

struct PostProcessor::Impl {
    GLuint shaderProgram = 0;
    int uCornerRadiusLoc = -1;
    int uShadowOffsetLoc = -1;
    int uBrightnessLoc = -1;
    int uContrastLoc = -1;
};

PostProcessor::PostProcessor() : impl_(std::make_unique<Impl>()) {}

PostProcessor::~PostProcessor() {
    cleanup();
}

bool PostProcessor::init(int width, int height) {
    if (initialized_) {
        return true;
    }
    
    width_ = width;
    height_ = height;
    
    // 创建 FBO
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    
    // 创建纹理
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D, texture_, 0);
    
    // 创建 Renderbuffer（深度/模板）
    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_RENDERBUFFER, rbo_);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("FBO is not complete");
        cleanup();
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // 编译 Shader（使用圆角 Shader 作为默认）
    if (!createShader(sVertexShader, sFragmentRounded)) {
        LOG_ERROR("Failed to create shader");
        cleanup();
        return false;
    }
    
    // 获取 Uniform 位置
    impl_->uCornerRadiusLoc = glGetUniformLocation(shaderProgram_, "uCornerRadius");
    impl_->uShadowOffsetLoc = glGetUniformLocation(shaderProgram_, "uShadowOffset");
    impl_->uBrightnessLoc = glGetUniformLocation(shaderProgram_, "uBrightness");
    impl_->uContrastLoc = glGetUniformLocation(shaderProgram_, "uContrast");
    
    initialized_ = true;
    LOG_INFO("PostProcessor initialized: %dx%d", width, height);
    
    return true;
}

void PostProcessor::beginOffscreen() {
    bindFramebuffer();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::endOffscreen(Effect effect) {
    unbindFramebuffer();
    
    // 应用效果
    switch (effect) {
        case Effect::RoundedCorners:
            applyRoundedCorners(params_.cornerRadius);
            break;
        case Effect::Shadow:
            applyShadow(params_.shadowBlur, params_.shadowOffset);
            break;
        case Effect::Blur:
            applyBlur(params_.blurRadius);
            break;
        case Effect::Brightness:
        case Effect::Contrast:
            applyColorAdjust(params_.brightness, params_.contrast);
            break;
        case Effect::Grayscale:
            applyGrayscale();
            break;
        default:
            break;
    }
}

void PostProcessor::applyRoundedCorners(float radius) {
    if (!initialized_) return;
    
    glUseProgram(shaderProgram_);
    glUniform1f(impl_->uCornerRadiusLoc, radius);
    
    // TODO: 绘制带纹理的四边形
}

void PostProcessor::applyShadow(float blur, float offset) {
    if (!initialized_) return;
    
    glUseProgram(shaderProgram_);
    glUniform2f(impl_->uShadowOffsetLoc, offset, offset);
    
    // TODO: 绘制带阴影的四边形
}

void PostProcessor::applyBlur(float radius) {
    // TODO: 实现高斯模糊（需要多 pass）
    LOG_DEBUG("Blur effect requested: %.2f", radius);
}

void PostProcessor::applyColorAdjust(float brightness, float contrast) {
    if (!initialized_) return;
    
    glUseProgram(shaderProgram_);
    glUniform1f(impl_->uBrightnessLoc, brightness);
    glUniform1f(impl_->uContrastLoc, contrast);
    
    // TODO: 绘制带颜色调整的四边形
}

void PostProcessor::applyGrayscale() {
    // TODO: 实现灰度效果
    LOG_DEBUG("Grayscale effect applied");
}

void PostProcessor::resize(int width, int height) {
    if (width == width_ && height == height_) {
        return;
    }
    
    cleanup();
    init(width, height);
}

void PostProcessor::cleanup() {
    if (fbo_ != 0) {
        glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
    }
    
    if (texture_ != 0) {
        glDeleteTextures(1, &texture_);
        texture_ = 0;
    }
    
    if (rbo_ != 0) {
        glDeleteRenderbuffers(1, &rbo_);
        rbo_ = 0;
    }
    
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
    
    initialized_ = false;
}

bool PostProcessor::createShader(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    
    // 检查编译状态
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        LOG_ERROR("Vertex shader compilation failed: %s", infoLog);
        return false;
    }
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        LOG_ERROR("Fragment shader compilation failed: %s", infoLog);
        return false;
    }
    
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);
    
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram_, 512, nullptr, infoLog);
        LOG_ERROR("Shader program linking failed: %s", infoLog);
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

void PostProcessor::bindFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, width_, height_);
}

void PostProcessor::unbindFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
}

} // namespace Component
