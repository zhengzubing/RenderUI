#pragma once

#include <memory>
#include <mutex>
#include <cstdint>

namespace Component {

/**
 * @brief 视频流控件 - 每帧渲染
 * 
 * 支持 YUV420/YUV422 格式，GPU 颜色转换
 */
class VideoStream {
public:
    VideoStream() = default;
    ~VideoStream() = default;
    
    /**
     * @brief 推送视频帧
     * @param data YUV 数据
     * @param width 宽度
     * @param height 高度
     */
    void pushFrame(const uint8_t* yData, const uint8_t* uData, const uint8_t* vData, 
                   int width, int height);
    
    /**
     * @brief 是否需要渲染（始终返回 true）
     */
    bool needsRender() const { return true; }
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::mutex frameMutex_;
};

} // namespace Component
