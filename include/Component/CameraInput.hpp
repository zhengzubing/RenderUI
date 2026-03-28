#pragma once

#include <string>
#include <memory>
#include <functional>
#include <cstdint>

namespace Component {

class VideoStream;

/**
 * @brief 视频帧回调类型
 */
using FrameCallback = std::function<void(const uint8_t* yData, const uint8_t* uData, 
                                         const uint8_t* vData, int width, int height)>;

/**
 * @brief Camera 输入源
 * 
 * 从 V4L2 设备捕获视频帧，支持 YUV420/NV12 格式
 */
class CameraInput {
public:
    /**
     * @brief Camera 配置
     */
    struct Config {
        int deviceId = 0;          // /dev/videoN
        int width = 640;
        int height = 480;
        int fps = 30;
        std::string format = "YUV420";  // YUV420, NV12, MJPEG
    };
    
    explicit CameraInput(const Config& config = Config());
    ~CameraInput();
    
    /**
     * @brief 设置帧回调
     */
    void setFrameCallback(FrameCallback callback);
    
    /**
     * @brief 开始捕获
     * @return 是否成功
     */
    bool start();
    
    /**
     * @brief 停止捕获
     */
    void stop();
    
    /**
     * @brief 是否正在捕获
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief 获取配置
     */
    const Config& getConfig() const { return config_; }
    
    /**
     * @brief 设置配置
     */
    void setConfig(const Config& config);
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    
    Config config_;
    FrameCallback callback_;
    bool running_ = false;
    
    /**
     * @brief 从 V4L2 设备读取帧
     */
    bool readFrame();
    
    /**
     * @brief 处理帧数据并调用回调
     */
    void processFrame(uint8_t* data, size_t size);
};

} // namespace Component
