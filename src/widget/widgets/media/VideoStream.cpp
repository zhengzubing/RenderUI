#include "VideoStream.hpp"
#include "CairoGlRenderer.hpp"
#include "Logger.hpp"
#include <GLES3/gl3.h>
#include <cstring>
#include <chrono>
#include <fstream>
#include <sstream>

namespace Component {

// Shader 文件路径
static const std::string sVertexShaderPath = "shaders/videostream_vertex.glsl";
static const std::string sFragmentShaderPath = "shaders/videostream_fragment.glsl";

// 从文件加载 Shader 源码
static std::string loadShaderSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_E << "Failed to open shader file: " << path;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 编译 Shader 并检查错误
static GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    // 检查编译状态
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOG_E << "Shader compilation failed: " << log;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

// 链接 Shader 程序并检查错误
static GLuint linkShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // 检查链接状态
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        LOG_E << "Shader program linking failed: " << log;
        glDeleteProgram(program);
        return 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

VideoStream::VideoStream(const std::string& source) : source_(source) {}

VideoStream::~VideoStream() {
    stop();
    cleanupOpenGL();
}

void VideoStream::setSource(const std::string& source) {
    source_ = source;
    markDirty();
}

void VideoStream::start() {
    if (!playing_) {
        playing_ = true;
        initOpenGL();
        LOG_I << "VideoStream started: " << source_;
    }
}

void VideoStream::stop() {
    if (playing_) {
        playing_ = false;
        hasNewFrame_ = false;
        LOG_I << "VideoStream stopped";
    }
}

void VideoStream::pushFrame(const uint8_t* yData, const uint8_t* uData, const uint8_t* vData,
                            int width, int height) {
    if (!playing_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(frameMutex_);
    
    // 更新帧数据
    currentFrame_.width = width;
    currentFrame_.height = height;
    
    size_t ySize = width * height;
    size_t uvSize = ySize / 4;  // YUV420
    
    currentFrame_.yData.assign(yData, yData + ySize);
    currentFrame_.uData.assign(uData, uData + uvSize);
    currentFrame_.vData.assign(vData, vData + uvSize);
    
    hasNewFrame_ = true;
    
    // 计算 FPS
    auto now = std::chrono::steady_clock::now();
    frameCount_++;
    
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastFrameTime_).count();
    
    if (elapsed >= 1000) {
        fps_ = static_cast<float>(frameCount_) * 1000.0f / static_cast<float>(elapsed);
        frameCount_ = 0;
        lastFrameTime_ = now;
    }
    
    markDirty();
}

void VideoStream::pushFrameNV12(const uint8_t* yData, const uint8_t* uvData, 
                                 int width, int height) {
    if (!playing_) {
        return;
    }
    
    // NV12 转 YUV420
    size_t ySize = width * height;
    size_t uvSize = ySize / 2;
    
    std::vector<uint8_t> uData(uvSize / 2);
    std::vector<uint8_t> vData(uvSize / 2);
    
    // 分离 UV 交错数据
    for (size_t i = 0; i < uvSize / 2; i++) {
        uData[i] = uvData[i * 2];
        vData[i] = uvData[i * 2 + 1];
    }
    
    pushFrame(yData, uData.data(), vData.data(), width, height);
}

void VideoStream::initOpenGL() {
    if (initialized_) {
        return;
    }
    
    // 创建纹理
    glGenTextures(3, textures_);
    
    for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, textures_[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    // 加载并编译 Shader
    std::string vertexSource = loadShaderSource(sVertexShaderPath);
    std::string fragmentSource = loadShaderSource(sFragmentShaderPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        LOG_E << "Failed to load shader sources";
        return;
    }
    
    // 编译 Shader
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        LOG_E << "Failed to compile shaders";
        return;
    }
    
    // 链接 Shader 程序
    shaderProgram_ = linkShaderProgram(vertexShader, fragmentShader);
    
    if (shaderProgram_ == 0) {
        LOG_E << "Failed to link shader program";
        return;
    }
    
    initialized_ = true;
    LOG_I << "VideoStream OpenGL initialized";
}

void VideoStream::updateTextures(const VideoFrame& frame) {
    if (!initialized_ || !frame.isValid()) {
        return;
    }
    
    int width = frame.width;
    int height = frame.height;
    
    // 更新 Y 纹理
    glBindTexture(GL_TEXTURE_2D, textures_[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, 
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, frame.yData.data());
    
    // 更新 U 纹理
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, frame.uData.data());
    
    // 更新 V 纹理
    glBindTexture(GL_TEXTURE_2D, textures_[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, frame.vData.data());
}

void VideoStream::renderWithOpenGL(Canvas& canvas) {
    if (!initialized_ || !hasNewFrame_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(frameMutex_);
    
    if (!currentFrame_.isValid()) {
        return;
    }
    
    // 更新纹理
    updateTextures(currentFrame_);
    
    // 使用 Shader 渲染
    glUseProgram(shaderProgram_);
    
    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_[0]);
    glUniform1i(glGetUniformLocation(shaderProgram_, "uTextureY"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_[1]);
    glUniform1i(glGetUniformLocation(shaderProgram_, "uTextureU"), 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures_[2]);
    glUniform1i(glGetUniformLocation(shaderProgram_, "uTextureV"), 2);
    
    // TODO: 绘制四边形覆盖控件区域
    // 这里需要使用 OpenGL 绘制一个覆盖控件区域的四边形
    
    hasNewFrame_ = false;
}

void VideoStream::onDraw(Canvas& canvas) {
    if (!playing_) {
        return;
    }
    
    // 使用 OpenGL 渲染视频帧
    renderWithOpenGL(canvas);
}

void VideoStream::cleanupOpenGL() {
    if (textures_[0] != 0) {
        glDeleteTextures(3, textures_);
        textures_[0] = textures_[1] = textures_[2] = 0;
    }
    
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
    
    initialized_ = false;
}

} // namespace Component
