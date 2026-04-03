#include "PostProcessor.hpp"
#include "Logger.hpp"
#include <GLES2/gl2ext.h>
#include <fstream>
#include <sstream>

namespace Component {

// Shader 文件路径
static const std::string sShaderPath = "shaders/";

// 从文件加载 Shader 源码
static std::string loadShaderSource(const std::string& filename) {
    std::string fullPath = sShaderPath + filename;
    std::ifstream file(fullPath);
    
    if (!file.is_open()) {
        LOG_E << "Failed to open shader file: " << fullPath;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

PostProcessor::PostProcessor() = default;

PostProcessor::~PostProcessor() {
    cleanup();
}

bool PostProcessor::init(int width, int height) {
    if (initialized_) {
        return true;
    }
    
    width_ = width;
    height_ = height;
    
    // FBO creation
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D, texture_, 0);
    
    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_RENDERBUFFER, rbo_);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_E << "FBO is not complete";
        cleanup();
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // 从文件加载 Shader（使用圆角 Shader 作为默认）
    std::string vertexSource = loadShaderSource("postprocessor_vertex.glsl");
    std::string fragmentSource = loadShaderSource("postprocessor_rounded.glsl");
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        LOG_E << "Failed to load shader sources";
        cleanup();
        return false;
    }
    
    if (!createShader(vertexSource.c_str(), fragmentSource.c_str())) {
        LOG_E << "Failed to create shader";
        cleanup();
        return false;
    }
    
    // 获取 Uniform 位置
    uCornerRadiusLoc_ = glGetUniformLocation(shaderProgram_, "uCornerRadius");
    uShadowOffsetLoc_ = glGetUniformLocation(shaderProgram_, "uShadowOffset");
    uBrightnessLoc_ = glGetUniformLocation(shaderProgram_, "uBrightness");
    uContrastLoc_ = glGetUniformLocation(shaderProgram_, "uContrast");
    
    initialized_ = true;
    LOG_I << "PostProcessor initialized: " << width << "x" << height;
    
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
    glUniform1f(uCornerRadiusLoc_, radius);
    
    // TODO: 绘制带纹理的四边形
}

void PostProcessor::applyShadow(float blur, float offset) {
    if (!initialized_) return;
    
    glUseProgram(shaderProgram_);
    glUniform2f(uShadowOffsetLoc_, offset, offset);
    
    // TODO: 绘制带阴影的四边形
}

void PostProcessor::applyBlur(float radius) {
    // TODO: 实现高斯模糊（需要多 pass）
    LOG_D << "Blur effect requested: " << radius;
}

void PostProcessor::applyColorAdjust(float brightness, float contrast) {
    if (!initialized_) return;
    
    glUseProgram(shaderProgram_);
    glUniform1f(uBrightnessLoc_, brightness);
    glUniform1f(uContrastLoc_, contrast);
    
    // TODO: 绘制带颜色调整的四边形
}

void PostProcessor::applyGrayscale() {
    // TODO: 实现灰度效果
    LOG_D << "Grayscale effect applied";
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
    
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        LOG_E << "Vertex shader compilation failed: " << infoLog;
        return false;
    }
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        LOG_E << "Fragment shader compilation failed: " << infoLog;
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
        LOG_E << "Shader program linking failed: " << infoLog;
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
