#include "Component/VideoStream.hpp"

namespace Component {

struct VideoStream::Impl {
    uint32_t textures[3] = {0};  // Y/U/V 纹理
    int width = 0;
    int height = 0;
    bool newFrameAvailable = false;
};

void VideoStream::pushFrame(const uint8_t* yData, const uint8_t* uData, const uint8_t* vData,
                            int width, int height) {
    std::lock_guard<std::mutex> lock(frameMutex_);
    
    if (!impl_) {
        impl_ = std::make_unique<Impl>();
    }
    
    impl_->width = width;
    impl_->height = height;
    impl_->newFrameAvailable = true;
    
    // TODO: 上传 YUV 纹理到 GPU
}

} // namespace Component
