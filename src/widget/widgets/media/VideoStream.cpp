#include "Component/VideoStream.hpp"
#include "Component/RenderContext.hpp"
#include "Component/Logger.hpp"
#include <GLES2/gl2.h>
#include <cstring>
#include <chrono>

namespace Component {

// YUV 到 RGB 的 OpenGL ES Shader
static const char* sVertexShaderSource = R"(
    attribute vec4 aPosition;
    attribute vec2 aTexCoord;
    varying vec2 vTexCoord;
    
    void main() {
        gl_Position = aPosition;
        vTexCoord = aTexCoord;
    }
)";

static const char* sFragmentShaderSourceYUV420 = R"(
    precision mediump float;
    varying vec2 vTexCoord;
    uniform sampler2D uTextureY;
    uniform sampler2D uTextureU;
    uniform sampler2D uTextureV;
    
    void main() {
        float y = texture2D(uTextureY, vTexCoord).r;
        float u = texture2D(uTextureU, vTexCoord).r - 0.5;
        float v = texture2D(uTextureV, vTexCoord).r - 0.5;
        
        // YUV420 到 RGB 转换矩阵
        float r = y + 1.402 * v;
        float g = y - 0.344 * u - 0.714 * v;
        float b = y + 1.772 * u;
        
        gl_FragColor = vec4(r, g, b, 1.0);
    }
)";

struct VideoStream::Impl {
    int width = 0;
    int height = 0;
    bool initialized = false;
};

VideoStream::VideoStream(const std::string& source) : source_(source) {
    impl_ = std::make_unique<Impl>();
}

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
        LOG_INFO("VideoStream started: %s", source_.c_str());
    }
}

void VideoStream::stop() {
    if (playing_) {
        playing_ = false;
        hasNewFrame_ = false;
        LOG_INFO("VideoStream stopped");
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
        now - std::chrono::milliseconds(lastFrameTime_)).count();
    
    if (elapsed >= 1000) {
        fps_ = static_cast<float>(frameCount_) * 1000.0f / static_cast<float>(elapsed);
        frameCount_ = 0;
        lastFrameTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
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
    if (impl_->initialized) {
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
    
    // 编译 Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &sVertexShaderSourceYUV420, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &sFragmentShaderSourceYUV420, nullptr);
    glCompileShader(fragmentShader);
    
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    impl_->initialized = true;
    LOG_INFO("VideoStream OpenGL initialized");
}

void VideoStream::updateTextures(const VideoFrame& frame) {
    if (!impl_->initialized || !frame.isValid()) {
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
    if (!impl_->initialized || !hasNewFrame_) {
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
    
    impl_->initialized = false;
}

} // namespace Component
