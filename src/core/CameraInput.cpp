#include "CameraInput.hpp"
#include "Logger.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <cstring>
#include <vector>

namespace Component {

struct CameraInput::Impl {
    int fd = -1;  // V4L2 设备文件描述符
    std::vector<void*> buffers;  // 内存映射缓冲区
    size_t numBuffers = 4;
};

CameraInput::CameraInput(const Config& config) : config_(config) {
    impl_ = std::make_unique<Impl>();
}

CameraInput::~CameraInput() {
    stop();
}

void CameraInput::setFrameCallback(FrameCallback callback) {
    callback_ = std::move(callback);
}

bool CameraInput::start() {
    if (running_) {
        return true;
    }
    
    // 打开 V4L2 设备
    std::string devicePath = "/dev/video" + std::to_string(config_.deviceId);
    impl_->fd = open(devicePath.c_str(), O_RDWR | O_NONBLOCK, 0);
    
    if (impl_->fd < 0) {
        LOG_ERROR("Failed to open camera device: %s", devicePath.c_str());
        return false;
    }
    
    // 设置格式
    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = config_.width;
    fmt.fmt.pix.height = config_.height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    
    if (ioctl(impl_->fd, VIDIOC_S_FMT, &fmt) < 0) {
        LOG_ERROR("Failed to set video format");
        close(impl_->fd);
        impl_->fd = -1;
        return false;
    }
    
    // 请求缓冲区
    struct v4l2_requestbuffers req = {};
    req.count = impl_->numBuffers;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(impl_->fd, VIDIOC_REQBUFS, &req) < 0) {
        LOG_ERROR("Failed to request buffers");
        close(impl_->fd);
        impl_->fd = -1;
        return false;
    }
    
    // 映射缓冲区
    impl_->buffers.resize(req.count);
    for (size_t i = 0; i < req.count; i++) {
        struct v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(impl_->fd, VIDIOC_QUERYBUF, &buf) < 0) {
            LOG_ERROR("Failed to query buffer");
            close(impl_->fd);
            return false;
        }
        
        impl_->buffers[i] = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, 
                                  MAP_SHARED, impl_->fd, buf.m.offset);
        
        if (impl_->buffers[i] == MAP_FAILED) {
            LOG_ERROR("Failed to mmap buffer");
            close(impl_->fd);
            return false;
        }
    }
    
    // 队列缓冲区
    for (size_t i = 0; i < req.count; i++) {
        struct v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(impl_->fd, VIDIOC_QBUF, &buf) < 0) {
            LOG_ERROR("Failed to queue buffer");
            close(impl_->fd);
            return false;
        }
    }
    
    // 开始流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(impl_->fd, VIDIOC_STREAMON, &type) < 0) {
        LOG_ERROR("Failed to start streaming");
        close(impl_->fd);
        return false;
    }
    
    running_ = true;
    LOG_INFO("Camera started: /dev/video%d %dx%d@%dfps", 
             config_.deviceId, config_.width, config_.height, config_.fps);
    
    return true;
}

void CameraInput::stop() {
    if (!running_) {
        return;
    }
    
    // 停止流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(impl_->fd, VIDIOC_STREAMOFF, &type);
    
    // 解除内存映射
    for (auto* buffer : impl_->buffers) {
        if (buffer) {
            munmap(buffer, 1);  // 简化处理
        }
    }
    
    // 关闭设备
    if (impl_->fd >= 0) {
        close(impl_->fd);
        impl_->fd = -1;
    }
    
    impl_->buffers.clear();
    running_ = false;
    
    LOG_INFO("Camera stopped");
}

void CameraInput::setConfig(const Config& config) {
    if (running_) {
        LOG_WARNING("Cannot change config while running");
        return;
    }
    config_ = config;
}

bool CameraInput::readFrame() {
    if (!running_ || impl_->fd < 0) {
        return false;
    }
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(impl_->fd, &fds);
    
    struct timeval tv = {};
    tv.tv_sec = 1;
    
    // 等待帧就绪
    int r = select(impl_->fd + 1, &fds, nullptr, nullptr, &tv);
    if (r <= 0) {
        return false;
    }
    
    // 出队帧
    struct v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(impl_->fd, VIDIOC_DQBUF, &buf) < 0) {
        return false;
    }
    
    // 处理帧数据
    processFrame(static_cast<uint8_t*>(impl_->buffers[buf.index]), buf.bytesused);
    
    // 重新入队
    if (ioctl(impl_->fd, VIDIOC_QBUF, &buf) < 0) {
        return false;
    }
    
    return true;
}

void CameraInput::processFrame(uint8_t* data, size_t size) {
    if (!callback_) {
        return;
    }
    
    int width = config_.width;
    int height = config_.height;
    size_t ySize = width * height;
    size_t uvSize = ySize / 4;
    
    // YUV420 格式：Y 平面 + U 平面 + V 平面
    uint8_t* yData = data;
    uint8_t* uData = data + ySize;
    uint8_t* vData = data + ySize + uvSize;
    
    callback_(yData, uData, vData, width, height);
}

} // namespace Component
