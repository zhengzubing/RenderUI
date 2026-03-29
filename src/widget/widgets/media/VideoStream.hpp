#pragma once

#include "Widget.hpp"
#include <memory>
#include <mutex>
#include <cstdint>
#include <string>

namespace Component {

/**
 * @brief 视频帧数据结构
 */
struct VideoFrame {
    std::vector<uint8_t> yData;
    std::vector<uint8_t> uData;
    std::vector<uint8_t> vData;
    int width = 0;
    int height = 0;
    bool isValid() const { return width > 0 && height > 0 && !yData.empty(); }
};

/**
 * @brief 视频流控件 - 每帧渲染
 * 
 * 支持 YUV420/YUV422 格式，使用 OpenGL ES Shader 进行 GPU 颜色转换
 * 从 Camera 或视频文件接收帧数据，实时渲染到屏幕
 */
class VideoStream : public Widget {
public:
    explicit VideoStream(const std::string& source = "");
    ~VideoStream() override;
    
    /**
     * @brief 设置视频源
     * @param source 视频源标识（如 "camera://0" 或 "file:///path/to/video.mp4"）
     */
    void setSource(const std::string& source);
    
    /**
     * @brief 获取视频源
     */
    const std::string& getSource() const { return source_; }
    
    /**
     * @brief 推送视频帧（YUV420 格式）
     * @param yData Y 分量数据
     * @param uData U 分量数据
     * @param vData V 分量数据
     * @param width 宽度
     * @param height 高度
     */
    void pushFrame(const uint8_t* yData, const uint8_t* uData, const uint8_t* vData,
                   int width, int height);
    
    /**
     * @brief 推送视频帧（NV12 格式）
     * @param yData Y 分量数据
     * @param uvData UV 交错数据
     * @param width 宽度
     * @param height 高度
     */
    void pushFrameNV12(const uint8_t* yData, const uint8_t* uvData, int width, int height);
    
    /**
     * @brief 是否需要渲染（视频流始终需要渲染）
     */
    bool needsRender() const override { return hasNewFrame_; }
    
    /**
     * @brief 开始视频流
     */
    void start();
    
    /**
     * @brief 停止视频流
     */
    void stop();
    
    /**
     * @brief 是否正在播放
     */
    bool isPlaying() const { return playing_; }
    
    /**
     * @brief 获取当前帧率
     */
    float getCurrentFps() const { return fps_; }
    
protected:
    void onDraw(Canvas& canvas) override;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    
    std::string source_;
    bool playing_ = false;
    bool hasNewFrame_ = false;
    float fps_ = 0.0f;
    
    uint64_t lastFrameTime_ = 0;
    int frameCount_ = 0;
    
    std::mutex frameMutex_;
    VideoFrame currentFrame_;
    
    // OpenGL 纹理 ID
    GLuint textures_[3] = {0};  // Y, U, V
    GLuint shaderProgram_ = 0;
    
    void initOpenGL();
    void updateTextures(const VideoFrame& frame);
    void renderWithOpenGL(Canvas& canvas);
    void cleanupOpenGL();
};

} // namespace Component
