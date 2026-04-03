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

CameraInput::CameraInput(const Config& config) : config_(config) {}

// 默认构造函数
CameraInput::CameraInput() : CameraInput(Config()) {}

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
    fd_ = open(devicePath.c_str(), O_RDWR | O_NONBLOCK, 0);
    
    if (fd_ < 0) {
        LOG_E << "Failed to open camera device: " << devicePath;
        return false;
    }
    
    // 设置格式
    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = config_.width;
    fmt.fmt.pix.height = config_.height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    
    if (ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
        LOG_E << "Failed to set video format";
        close(fd_);
        fd_ = -1;
        return false;
    }
    
    // 请求缓冲区
    struct v4l2_requestbuffers req = {};
    req.count = numBuffers_;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {
        LOG_E << "Failed to request buffers";
        close(fd_);
        fd_ = -1;
        return false;
    }
    
    // 映射缓冲区
    buffers_.resize(req.count);
    for (size_t i = 0; i < req.count; i++) {
        struct v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) < 0) {
            LOG_E << "Failed to query buffer";
            close(fd_);
            return false;
        }
        
        buffers_[i] = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, 
                              MAP_SHARED, fd_, buf.m.offset);
        
        if (buffers_[i] == MAP_FAILED) {
            LOG_E << "Failed to mmap buffer";
            close(fd_);
            return false;
        }
    }
    
    // 队列缓冲区
    for (size_t i = 0; i < req.count; i++) {
        struct v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
            LOG_E << "Failed to queue buffer";
            close(fd_);
            return false;
        }
    }
    
    // 开始流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
        LOG_E << "Failed to start streaming";
        close(fd_);
        return false;
    }
    
    running_ = true;
    LOG_I << "Camera started: /dev/video" << config_.deviceId << " " 
             << config_.width << "x" << config_.height << "@" << config_.fps << "fps";
    
    return true;
}

void CameraInput::stop() {
    if (!running_) {
        return;
    }
    
    // 停止流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd_, VIDIOC_STREAMOFF, &type);
    
    // 解除内存映射
    for (auto* buffer : buffers_) {
        if (buffer) {
            munmap(buffer, 1);  // 简化处理
        }
    }
    
    // 关闭设备
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
    
    buffers_.clear();
    running_ = false;
    
    LOG_I << "Camera stopped";
}

void CameraInput::setConfig(const Config& config) {
    if (running_) {
        LOG_W << "Cannot change config while running";
        return;
    }
    config_ = config;
}

bool CameraInput::readFrame() {
    if (!running_ || fd_ < 0) {
        return false;
    }
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);
    
    struct timeval tv = {};
    tv.tv_sec = 1;
    
    // 等待帧就绪
    int r = select(fd_ + 1, &fds, nullptr, nullptr, &tv);
    if (r <= 0) {
        return false;
    }
    
    // 出队帧
    struct v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
        return false;
    }
    
    // 处理帧数据
    processFrame(static_cast<uint8_t*>(buffers_[buf.index]), buf.bytesused);
    
    // 重新入队
    if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
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
