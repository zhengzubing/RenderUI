#pragma once

#include "Component/RenderContext.hpp"
#include <memory>
#include <string>

// Skia 前向声明
class SkSurface;
class SkCanvas;
class SkPaint;
class SkFont;

namespace Component {

/**
 * @brief Skia 渲染器
 * 
 * 使用 Skia 进行 GPU 加速的 2D 渲染（阶段四）
 * 替代 Cairo 提供更高的性能
 */
class SkiaRenderer {
public:
    SkiaRenderer();
    ~SkiaRenderer();
    
    /**
     * @brief 初始化 Skia 渲染器
     * @param width 宽度
     * @param height 高度
     * @param useGpu 是否使用 GPU 加速
     * @return 是否成功
     */
    bool init(int width, int height, bool useGpu = true);
    
    /**
     * @brief 获取 Skia 画布
     */
    SkCanvas* getCanvas() const;
    
    /**
     * @brief 获取 Skia 表面
     */
    SkSurface* getSurface() const;
    
    /**
     * @brief 开始新帧
     */
    void beginFrame();
    
    /**
     * @brief 结束帧并提交渲染
     */
    void endFrame();
    
    /**
     * @brief 调整大小
     */
    void resize(int width, int height);
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
    /**
     * @brief 是否已初始化
     */
    bool isInitialized() const { return initialized_; }
    
    /**
     * @brief 是否使用 GPU
     */
    bool isUsingGPU() const { return useGpu_; }
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    
    bool initialized_ = false;
    bool useGpu_ = true;
    int width_ = 0;
    int height_ = 0;
};

/**
 * @brief Skia 画笔封装
 */
class SkiaPaint {
public:
    SkiaPaint();
    ~SkiaPaint();
    
    /**
     * @brief 设置颜色
     */
    void setColor(const Color& color);
    
    /**
     * @brief 设置线宽
     */
    void setStrokeWidth(float width);
    
    /**
     * @brief 设置抗锯齿
     */
    void setAntiAlias(bool aa);
    
    /**
     * @brief 设置为填充模式
     */
    void setFill();
    
    /**
     * @brief 设置为描边模式
     */
    void setStroke();
    
    /**
     * @brief 获取底层 SkPaint
     */
    SkPaint* getSkPaint() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Skia 字体封装
 */
class SkiaFont {
public:
    SkiaFont(const std::string& fontFamily = "Sans", float size = 14.0f);
    ~SkiaFont();
    
    /**
     * @brief 设置字体家族
     */
    void setFontFamily(const std::string& family);
    
    /**
     * @brief 设置字体大小
     */
    void setSize(float size);
    
    /**
     * @brief 获取底层 SkFont
     */
    SkFont* getSkFont() const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Component
