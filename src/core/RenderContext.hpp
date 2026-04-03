#pragma once

#include "Types.hpp"
#include <cairo/cairo.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <memory>

namespace Component {

/**
 * @brief 渲染上下文 - 封装 Cairo 和 OpenGL 操作
 * 
 * 提供统一的绘图接口，底层支持 Cairo 2D 绘制和 OpenGL 纹理上传
 */
class RenderContext {
public:
    RenderContext();
    ~RenderContext();
    
    /**
     * @brief 初始化 EGL 和 Cairo
     * @param display EGL 显示连接
     * @param surface EGL 表面
     * @return 是否成功
     */
    bool init(EGLDisplay display, EGLSurface surface);
    
    /**
     * @brief 获取 Cairo 绘图上下文
     */
    cairo_t* getCairoContext() const;
    
    /**
     * @brief 获取 EGL 表面
     */
    EGLSurface getEglSurface() const;
    
    /**
     * @brief 获取 EGL 显示连接
     */
    EGLDisplay getEglDisplay() const;
    
    /**
     * @brief 开始新帧
     */
    void beginFrame();
    
    /**
     * @brief 提交渲染并交换缓冲区
     */
    void endFrame();
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLSurface surface_ = EGL_NO_SURFACE;
    cairo_t* cairo_ = nullptr;
    cairo_surface_t* cairoSurface_ = nullptr;
    bool initialized_ = false;
};

/**
 * @brief Canvas - Cairo 绘图封装
 * 
 * 提供高级绘图接口
 */
class Canvas {
public:
    explicit Canvas(cairo_t* cr);
    ~Canvas() = default;
    
    /**
     * @brief 设置绘制颜色
     */
    void setColor(const Color& color);
    
    /**
     * @brief 绘制矩形
     */
    void drawRect(float x, float y, float width, float height, const Color& color);
    
    /**
     * @brief 填充矩形
     */
    void fillRect(float x, float y, float width, float height, const Color& color);
    
    /**
     * @brief 绘制文本
     */
    void drawText(float x, float y, const char* text, const char* font, float size, const Color& color);
    
    /**
     * @brief 绘制图片
     */
    void drawImage(float x, float y, float width, float height, cairo_surface_t* surface);
    
    /**
     * @brief 绘制圆角矩形
     */
    void drawRoundedRect(float x, float y, float width, float height, float radius, const Color& color);
    
    /**
     * @brief 获取底层 Cairo 上下文
     */
    cairo_t* getCairoContext() const { return cr_; }
    
private:
    cairo_t* cr_;
};

} // namespace Component
