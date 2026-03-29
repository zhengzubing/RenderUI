/* GTEST DISABLED - src/test_video_stream.cpp */
// #include <gtest/gtest.h>
#include "Component/VideoStream.hpp"
#include "Component/Types.hpp"
#include <thread>
#include <chrono>

using namespace Component;

/**
 * @brief VideoStream 视频流控件测试
 * 
 * 测试 YUV 视频流的帧推送、格式转换和渲染触发机制
 */

TEST(VideoStreamTest, CreateAndDestroy) {
    auto stream = std::make_shared<VideoStream>("test_stream");
    EXPECT_NE(stream, nullptr);
    // VideoStream 构造函数接受 source 参数，使用 getSource 检查
    EXPECT_EQ(stream->getSource(), "test_stream");
    
    // 默认状态检查
    EXPECT_FALSE(stream->isPlaying());
    EXPECT_TRUE(stream->isVisible());
}

TEST(VideoStreamTest, StartStopPlayback) {
    auto stream = std::make_shared<VideoStream>("camera://0");
    
    // 启动视频流
    bool started = stream->start();
    EXPECT_TRUE(started);
    EXPECT_TRUE(stream->isPlaying());
    
    // 停止视频流
    stream->stop();
    EXPECT_FALSE(stream->isPlaying());
}

TEST(VideoStreamTest, PushYUV420Frame) {
    auto stream = std::make_shared<VideoStream>("test_yuv420");
    stream->start();
    
    // 准备 YUV420 数据 (640x480)
    const int width = 640;
    const int height = 480;
    std::vector<uint8_t> yData(width * height);
    std::vector<uint8_t> uData(width * height / 4);
    std::vector<uint8_t> vData(width * height / 4);
    
    // 填充测试数据
    std::fill(yData.begin(), yData.end(), 128);
    std::fill(uData.begin(), uData.end(), 128);
    std::fill(vData.begin(), vData.end(), 128);
    
    // 推送帧
    stream->pushFrame(yData.data(), uData.data(), vData.data(), width, height);
    
    // 验证有新帧
    EXPECT_TRUE(stream->needsRender());
    
    stream->stop();
}

TEST(VideoStreamTest, PushNV12Frame) {
    auto stream = std::make_shared<VideoStream>("test_nv12");
    stream->start();
    
    // 准备 NV12 数据 (640x480)
    const int width = 640;
    const int height = 480;
    std::vector<uint8_t> yData(width * height);
    std::vector<uint8_t> uvData(width * height / 2);
    
    // 填充测试数据
    std::fill(yData.begin(), yData.end(), 128);
    std::fill(uvData.begin(), uvData.end(), 128);
    
    // 推送帧
    stream->pushFrameNV12(yData.data(), uvData.data(), width, height);
    
    // 验证有新帧
    EXPECT_TRUE(stream->needsRender());
    
    stream->stop();
}

TEST(VideoStreamTest, SetSource) {
    auto stream = std::make_shared<VideoStream>();
    
    // 设置相机源
    stream->setSource("camera://0");
    EXPECT_EQ(stream->getSource(), "camera://0");
    
    // 设置视频文件源
    stream->setSource("file:///path/to/video.mp4");
    EXPECT_EQ(stream->getSource(), "file:///path/to/video.mp4");
}

TEST(VideoStreamTest, FormatConversion) {
    // 测试不同 YUV 格式的接受能力
    auto stream = std::make_shared<VideoStream>("test_format");
    stream->start();
    
    const int width = 320;
    const int height = 240;
    
    // YUV420
    std::vector<uint8_t> y420(width * height);
    std::vector<uint8_t> u420(width * height / 4);
    std::vector<uint8_t> v420(width * height / 4);
    EXPECT_NO_THROW(stream->pushFrame(y420.data(), u420.data(), v420.data(), width, height));
    
    // YUV422
    std::vector<uint8_t> y422(width * height);
    std::vector<uint8_t> u422(width * height / 2);
    std::vector<uint8_t> v422(width * height / 2);
    EXPECT_NO_THROW(stream->pushFrameYUV422(y422.data(), u422.data(), v422.data(), width, height));
    
    // NV12
    std::vector<uint8_t> y12(width * height);
    std::vector<uint8_t> uv12(width * height / 2);
    EXPECT_NO_THROW(stream->pushFrameNV12(y12.data(), uv12.data(), width, height));
    
    stream->stop();
}

TEST(VideoStreamTest, FrameCallback) {
    auto stream = std::make_shared<VideoStream>("test_callback");
    
    bool callbackCalled = false;
    int callbackFrameCount = 0;
    
    // 设置帧回调
    stream->setFrameCallback([&](const uint8_t* y, const uint8_t* u, const uint8_t* v, 
                                  int w, int h) {
        callbackCalled = true;
        callbackFrameCount++;
        EXPECT_EQ(w, 640);
        EXPECT_EQ(h, 480);
    });
    
    stream->start();
    
    // 推送多帧
    for (int i = 0; i < 5; i++) {
        std::vector<uint8_t> y(640 * 480);
        std::vector<uint8_t> u(640 * 480 / 4);
        std::vector<uint8_t> v(640 * 480 / 4);
        stream->pushFrame(y.data(), u.data(), v.data(), 640, 480);
        
        // 给一点时间让回调执行
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    stream->stop();
    
    // 验证回调被调用
    EXPECT_TRUE(callbackCalled);
    EXPECT_GE(callbackFrameCount, 1);
}

TEST(VideoStreamTest, RotationSetting) {
    auto stream = std::make_shared<VideoStream>("test_rotation");
    
    // 设置旋转角度
    stream->setRotation(90.0f);
    EXPECT_FLOAT_EQ(stream->getRotation(), 90.0f);
    
    stream->setRotation(180.0f);
    EXPECT_FLOAT_EQ(stream->getRotation(), 180.0f);
    
    stream->setRotation(270.0f);
    EXPECT_FLOAT_EQ(stream->getRotation(), 270.0f);
}

TEST(VideoStreamTest, ScalingSetting) {
    auto stream = std::make_shared<VideoStream>("test_scaling");
    
    // 设置缩放模式
    stream->setScaleMode(VideoStream::ScaleMode::FIT);
    EXPECT_EQ(stream->getScaleMode(), VideoStream::ScaleMode::FIT);
    
    stream->setScaleMode(VideoStream::ScaleMode::FILL);
    EXPECT_EQ(stream->getScaleMode(), VideoStream::ScaleMode::FILL);
    
    stream->setScaleMode(VideoStream::ScaleMode::STRETCH);
    EXPECT_EQ(stream->getScaleMode(), VideoStream::ScaleMode::STRETCH);
}

TEST(VideoStreamTest, BrightnessContrast) {
    auto stream = std::make_shared<VideoStream>("test_bc");
    
    // 测试亮度调节
    stream->setBrightness(1.2f);
    EXPECT_FLOAT_EQ(stream->getBrightness(), 1.2f);
    
    // 测试对比度调节
    stream->setContrast(1.5f);
    EXPECT_FLOAT_EQ(stream->getContrast(), 1.5f);
    
    // 恢复默认值
    stream->setBrightness(1.0f);
    stream->setContrast(1.0f);
    EXPECT_FLOAT_EQ(stream->getBrightness(), 1.0f);
    EXPECT_FLOAT_EQ(stream->getContrast(), 1.0f);
}
