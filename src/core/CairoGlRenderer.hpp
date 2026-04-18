#pragma once

#include "Types.hpp"
#include <cairo/cairo.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <memory>
#include <unordered_map>
#include <map>
#include <string>

namespace Component {

/**
 * @brief Widget 纹理信息
 */
struct WidgetTexture {
    GLuint textureId = 0;
    cairo_surface_t* surface = nullptr;
    cairo_t* cairo = nullptr;
    int width = 0;
    int height = 0;
    bool dirty = true;  // 是否需要重新渲染
    
    // 屏幕位置（用于合成）
    float screenX = 0;
    float screenY = 0;
    
    // Z 序（值越大越靠上）
    int zIndex = 0;
};

/**
 * @brief 渲染上下文 - 封装 Cairo 和 OpenGL 操作
 * 
 * 多纹理架构：每个 Widget 有独立的 Cairo Surface 和 OpenGL 纹理
 */
class CairoGlRenderer {
public:
    CairoGlRenderer();
    ~CairoGlRenderer();
    
    /**
     * @brief 初始化 EGL
     * @param display EGL 显示连接
     * @param surface EGL 表面
     * @return 是否成功
     */
    bool init(EGLDisplay display, EGLSurface surface);
    
    /**
     * @brief 为 Widget 创建或获取渲染上下文
     * @param widgetId Widget 的唯一标识
     * @param width Widget 宽度
     * @param height Widget 高度
     * @return Cairo 绘图上下文
     */
    cairo_t* getCairoContext(const std::string& widgetId, int width, int height);
    
    /**
     * @brief 标记 Widget 需要重新渲染
     */
    void markWidgetDirty(const std::string& widgetId);
    
    /**
     * @brief 获取 Widget 的纹理 ID（如果未上传则先上传）
     */
    GLuint getWidgetTexture(const std::string& widgetId);
    
    /**
     * @brief 获取 EGL 表面
     */
    EGLSurface getEglSurface() const { return surface_; }
    
    /**
     * @brief 获取 EGL 显示连接
     */
    EGLDisplay getEglDisplay() const { return display_; }
    
    /**
     * @brief 开始新帧
     */
    void beginFrame();
    
    /**
     * @brief 更新 Widget 的位置信息（用于渲染时合成）
     */
    void updateWidgetPosition(const std::string& widgetId, float x, float y, float width, float height);
    
    /**
     * @brief 更新 Widget 的 Z 序（用于渲染顺序）
     * @note 会自动维护 Z 序容器的排序
     */
    void updateWidgetZIndex(const std::string& widgetId, int zIndex);
    
    /**
     * @brief 提交渲染并交换缓冲区
     * @note 合成所有 Widget 纹理到屏幕
     */
    void endFrame();
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
    /**
     * @brief 清理特定 Widget 的资源
     */
    void cleanupWidget(const std::string& widgetId);
    
private:
    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLSurface surface_ = EGL_NO_SURFACE;
    bool initialized_ = false;
    
    // 主存储：widgetId -> WidgetTexture（快速查找）
    std::unordered_map<std::string, WidgetTexture> widgetTextures_;
    
    // Z 序索引：zIndex -> widgetId（自动排序，支持多层）
    std::multimap<int, std::string> zOrderedWidgets_;
    
    // OpenGL 资源
    GLuint shaderProgram_ = 0;
    GLuint vbo_ = 0;
    
    int screenWidth_ = 0;
    int screenHeight_ = 0;
    
    /**
     * @brief 初始化着色器程序
     */
    GLuint createShaderProgram();
    
    /**
     * @brief 初始化 VBO
     */
    void initVBO();
    
    /**
     * @brief 从 Z 序容器中移除 Widget
     */
    void removeFromZOrder(const std::string& widgetId, int zIndex);
    
    /**
     * @brief 添加 Widget 到 Z 序容器
     */
    void addToZOrder(const std::string& widgetId, int zIndex);
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
