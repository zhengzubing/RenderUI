#include "CairoGlRenderer.hpp"
#include "Logger.hpp"
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>

namespace Component {

CairoGlRenderer::CairoGlRenderer() = default;

CairoGlRenderer::~CairoGlRenderer() {
    cleanup();
}

bool CairoGlRenderer::init(EGLDisplay display, EGLSurface surface) {
    display_ = display;
    surface_ = surface;
    
    if (display_ == EGL_NO_DISPLAY || surface_ == EGL_NO_SURFACE) {
        LOG_E << "Invalid EGL display or surface";
        return false;
    }
    
    // 获取屏幕尺寸
    EGLint w, h;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &w);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &h);
    screenWidth_ = w;
    screenHeight_ = h;
    
    LOG_I << "CairoGlRenderer initialized: " << screenWidth_ << "x" << screenHeight_;
    
    // 初始化 OpenGL 资源
    shaderProgram_ = createShaderProgram();
    initVBO();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    initialized_ = true;
    return true;
}

cairo_t* CairoGlRenderer::getCairoContext(const std::string& widgetId, int width, int height) {
    if (!initialized_) {
        LOG_E << "Renderer not initialized";
        return nullptr;
    }
    
    // 检查是否已存在
    auto it = widgetTextures_.find(widgetId);
    if (it != widgetTextures_.end()) {
        auto& wt = it->second;
        
        // 如果尺寸变化或 Cairo 上下文不存在，重新创建
        if (wt.width != width || wt.height != height || !wt.cairo || !wt.surface) {
            LOG_D << "Widget " << widgetId << " size changed or invalid: " 
                  << wt.width << "x" << wt.height << " -> " << width << "x" << height;
            
            // 清理旧资源
            if (wt.cairo) cairo_destroy(wt.cairo);
            if (wt.surface) cairo_surface_destroy(wt.surface);
            if (wt.textureId) glDeleteTextures(1, &wt.textureId);
            
            // 创建新资源
            wt.surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
            wt.cairo = cairo_create(wt.surface);
            wt.width = width;
            wt.height = height;
            wt.dirty = true;
            
            // 清除为透明背景
            cairo_save(wt.cairo);
            cairo_set_operator(wt.cairo, CAIRO_OPERATOR_CLEAR);
            cairo_paint(wt.cairo);
            cairo_restore(wt.cairo);
        }
        
        return wt.cairo;
    }
    
    // 创建新的 Widget 纹理（直接在 map 中构造，避免拷贝）
    LOG_D << "Creating new widget texture: " << widgetId << " (" << width << "x" << height << ")";
    
    auto& wt = widgetTextures_[widgetId];
    wt.width = width;
    wt.height = height;
    wt.surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    wt.cairo = cairo_create(wt.surface);
    wt.dirty = true;
    wt.zIndex = 0;  // 默认 Z 序
    
    // 清除为透明背景
    cairo_save(wt.cairo);
    cairo_set_operator(wt.cairo, CAIRO_OPERATOR_CLEAR);
    cairo_paint(wt.cairo);
    cairo_restore(wt.cairo);
    
    // 添加到 Z 序容器
    addToZOrder(widgetId, wt.zIndex);
    
    return wt.cairo;
}

void CairoGlRenderer::markWidgetDirty(const std::string& widgetId) {
    auto it = widgetTextures_.find(widgetId);
    if (it != widgetTextures_.end()) {
        it->second.dirty = true;
        LOG_D << "Widget marked dirty: " << widgetId;
    }
}

void CairoGlRenderer::updateWidgetPosition(const std::string& widgetId, float x, float y, float width, float height) {
    auto it = widgetTextures_.find(widgetId);
    if (it != widgetTextures_.end()) {
        LOG_D << "Updating widget position: " << widgetId << " (" << x << ", " << y << ") (" << width << "x" << height << ")";
        it->second.screenX = x;
        it->second.screenY = y;
        // 更新渲染尺寸（可能与 Cairo surface 尺寸不同）
        it->second.width = static_cast<int>(width);
        it->second.height = static_cast<int>(height);
    }
    // 如果不存在，不做任何操作，等待 getCairoContext 创建
}

void CairoGlRenderer::updateWidgetZIndex(const std::string& widgetId, int zIndex) {
    auto it = widgetTextures_.find(widgetId);
    if (it != widgetTextures_.end()) {
        auto& wt = it->second;
        
        LOG_I << "Updating widget Z index: " << widgetId << " " << wt.zIndex << " -> " << zIndex;


        // 如果 zIndex 没有变化，直接返回
        if (wt.zIndex == zIndex) {
            return;
        }
                
        // 从旧的 Z 序位置移除
        removeFromZOrder(widgetId, wt.zIndex);
        
        // 更新 zIndex
        wt.zIndex = zIndex;
        
        // 添加到新的 Z 序位置
        addToZOrder(widgetId, wt.zIndex);
    } else {
        LOG_W << "Widget not found for Z index update: " << widgetId;
    }
}

void CairoGlRenderer::removeFromZOrder(const std::string& widgetId, int zIndex) {
    auto range = zOrderedWidgets_.equal_range(zIndex);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == widgetId) {
            zOrderedWidgets_.erase(it);
            LOG_D << "Removed from Z order: " << widgetId << " (z=" << zIndex << ")";
            break;
        }
    }
}

void CairoGlRenderer::addToZOrder(const std::string& widgetId, int zIndex) {
    zOrderedWidgets_.emplace(zIndex, widgetId);
    LOG_D << "Added to Z order: " << widgetId << " (z=" << zIndex << ")";
}

GLuint CairoGlRenderer::getWidgetTexture(const std::string& widgetId) {
    auto it = widgetTextures_.find(widgetId);
    if (it == widgetTextures_.end()) {
        LOG_W << "Widget texture not found: " << widgetId;
        return 0;
    }
    
    auto& wt = it->second;
    
    // 如果纹理未创建或需要更新
    if (wt.textureId == 0 || wt.dirty) {
        // 确保 Cairo 内容已刷新
        cairo_surface_flush(wt.surface);
        
        unsigned char* data = cairo_image_surface_get_data(wt.surface);
        if (!data) {
            LOG_E << "Failed to get Cairo surface data for widget: " << widgetId;
            return 0;
        }
        
        // 创建或更新 OpenGL 纹理
        if (wt.textureId == 0) {
            glGenTextures(1, &wt.textureId);
            glBindTexture(GL_TEXTURE_2D, wt.textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wt.width, wt.height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, data);
            LOG_I << "wt.width: " << wt.width << " wt.height: " << wt.height << " wt.textureId: " << wt.textureId;
        } else {
            glBindTexture(GL_TEXTURE_2D, wt.textureId);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, wt.width, wt.height,
                GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        
        wt.dirty = false;
        LOG_D << "Texture updated for widget: " << widgetId;
    }
    
    return wt.textureId;
}

void CairoGlRenderer::beginFrame() {
    if (!initialized_) {
        return;
    }
    
    // 清除颜色缓冲区
    glViewport(0, 0, screenWidth_, screenHeight_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void CairoGlRenderer::endFrame() {
    if (!initialized_) {
        return;
    }
    
    // 使用着色器程序
    glUseProgram(shaderProgram_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    
    LOG_I << "Rendering frame: " << screenWidth_ << "x" << screenHeight_ 
          << " (widgets: " << zOrderedWidgets_.size() << ")";
    
    // 按 Z 序遍历（multimap 已经自动按 zIndex 升序排序）
    // Layer -N: zIndex < 0 (底层，如背景)
    // Layer 0:  zIndex = 0 (默认层，相同 zIndex 按插入顺序)
    // Layer +N: zIndex > 0 (顶层，如弹窗、提示)
    for (const auto& [zIndex, widgetId] : zOrderedWidgets_) {
        auto it = widgetTextures_.find(widgetId);
        if (it == widgetTextures_.end()) {
            LOG_W << "Widget texture not found in map: " << widgetId;
            continue;
        }
        
        auto& wt = it->second;
        
        // 获取或上传纹理
        GLuint texId = getWidgetTexture(widgetId);
        if (texId == 0) {
            LOG_W << "Failed to get texture for widget: " << widgetId;
            continue;
        }
        
        // 计算视口（注意：OpenGL Y 轴向上，屏幕坐标 Y 轴向下）
        GLint viewportX = static_cast<GLint>(wt.screenX);
        GLint viewportY = static_cast<GLint>(screenHeight_ - wt.screenY - wt.height);
        GLsizei viewportW = static_cast<GLsizei>(wt.width);
        GLsizei viewportH = static_cast<GLsizei>(wt.height);
        
        LOG_D << "Drawing widget (z=" << zIndex << "): " << widgetId 
              << " at (" << viewportX << ", " << viewportY << ") "
              << "size: " << viewportW << "x" << viewportH;
        
        // 设置视口和绘制区域
        glViewport(viewportX, viewportY, viewportW, viewportH);
        
        // 绑定纹理并绘制
        glBindTexture(GL_TEXTURE_2D, texId);
        
        // 检查 OpenGL 错误
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOG_E << "OpenGL error before draw " << widgetId << ": 0x" << std::hex << error;
        }
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // 检查绘制后的错误
        error = glGetError();
        if (error != GL_NO_ERROR) {
            LOG_E << "OpenGL error after draw " << widgetId << ": 0x" << std::hex << error;
        }
    }
    
    // 重置视口
    glViewport(0, 0, screenWidth_, screenHeight_);
    
    // 交换缓冲区
    eglSwapBuffers(display_, surface_);
}

void CairoGlRenderer::cleanup() {
    // 清理 Z 序容器
    zOrderedWidgets_.clear();
    
    // 清理所有 Widget 纹理
    for (auto& [id, wt] : widgetTextures_) {
        if (wt.cairo) cairo_destroy(wt.cairo);
        if (wt.surface) cairo_surface_destroy(wt.surface);
        if (wt.textureId) glDeleteTextures(1, &wt.textureId);
    }
    widgetTextures_.clear();
    
    // 清理 OpenGL 资源
    if (shaderProgram_) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
    
    if (vbo_) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    
    initialized_ = false;
}

void CairoGlRenderer::cleanupWidget(const std::string& widgetId) {
    auto it = widgetTextures_.find(widgetId);
    if (it != widgetTextures_.end()) {
        auto& wt = it->second;
        
        // 从 Z 序容器中移除
        removeFromZOrder(widgetId, wt.zIndex);
        
        if (wt.cairo) cairo_destroy(wt.cairo);
        if (wt.surface) cairo_surface_destroy(wt.surface);
        if (wt.textureId) glDeleteTextures(1, &wt.textureId);
        widgetTextures_.erase(it);
        LOG_D << "Widget cleaned up: " << widgetId;
    }
}

GLuint CairoGlRenderer::createShaderProgram() {
    const char* vs = R"(
        attribute vec2 aPos;
        attribute vec2 aTex;
        varying vec2 vTex;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            vTex = aTex;
        }
    )";
    
    const char* fs = R"(
        precision mediump float;
        uniform sampler2D uTex;
        varying vec2 vTex;
        void main() {
            gl_FragColor = texture2D(uTex, vTex);
        }
    )";
    
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vs, nullptr);
    glCompileShader(vertShader);
    
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fs, nullptr);
    glCompileShader(fragShader);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    return program;
}

void CairoGlRenderer::initVBO() {
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    
    // 全屏四边形顶点数据
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性
    GLint aPos = glGetAttribLocation(shaderProgram_, "aPos");
    GLint aTex = glGetAttribLocation(shaderProgram_, "aTex");
    
    glEnableVertexAttribArray(aPos);
    glVertexAttribPointer(aPos, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(aTex);
    glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

// ============================================================================
// Canvas 实现
// ============================================================================

Canvas::Canvas(cairo_t* cr) : cr_(cr) {}

void Canvas::setColor(const Color& color) {
    if (!cr_) return;
    cairo_set_source_rgba(cr_, color.r, color.g, color.b, 1.0);
}

void Canvas::drawRect(float x, float y, float width, float height, const Color& color) {
    if (!cr_) return;
    setColor(color);
    cairo_rectangle(cr_, x, y, width, height);
    cairo_stroke(cr_);
}

void Canvas::fillRect(float x, float y, float width, float height, const Color& color) {
    if (!cr_) return;
    setColor(color);
    cairo_rectangle(cr_, x, y, width, height);
    cairo_fill(cr_);
}

void Canvas::drawText(float x, float y, const char* text, const char* font, float size, const Color& color) {
    if (!cr_ || !text) return;
    
    setColor(color);
    
    char fontDesc[256];
    snprintf(fontDesc, sizeof(fontDesc), "%s %.1f", font ? font : "Sans", size);
    cairo_select_font_face(cr_, fontDesc, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    
    cairo_move_to(cr_, x, y);
    cairo_show_text(cr_, text);
    cairo_stroke(cr_);
}

void Canvas::drawImage(float x, float y, float width, float height, cairo_surface_t* surface) {
    if (!cr_ || !surface) return;
    
    int imgWidth = cairo_image_surface_get_width(surface);
    int imgHeight = cairo_image_surface_get_height(surface);
    
    if (imgWidth <= 0 || imgHeight <= 0) return;
    
    cairo_save(cr_);
    
    // 创建缩放后的图案
    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(surface);
    cairo_pattern_set_filter(pattern, CAIRO_FILTER_BEST);
    
    // 设置图案的矩阵变换以实现缩放
    cairo_matrix_t matrix;
    cairo_matrix_init_scale(&matrix, 
                           static_cast<double>(imgWidth) / width,
                           static_cast<double>(imgHeight) / height);
    cairo_pattern_set_matrix(pattern, &matrix);
    
    // 使用图案填充矩形区域（始终从 0,0 开始绘制）
    cairo_set_source(cr_, pattern);
    cairo_rectangle(cr_, 0, 0, width, height);
    cairo_fill(cr_);
    
    cairo_pattern_destroy(pattern);
    cairo_restore(cr_);
}

void Canvas::drawRoundedRect(float x, float y, float width, float height, float radius, const Color& color) {
    if (!cr_) return;
    
    cairo_new_path(cr_);
    
    // 绘制圆角矩形路径
    cairo_arc(cr_, x + radius, y + radius, radius, M_PI, 3 * M_PI / 2);
    cairo_arc(cr_, x + width - radius, y + radius, radius, 3 * M_PI / 2, 2 * M_PI);
    cairo_arc(cr_, x + width - radius, y + height - radius, radius, 0, M_PI / 2);
    cairo_arc(cr_, x + radius, y + height - radius, radius, M_PI / 2, M_PI);
    cairo_close_path(cr_);
    
    setColor(color);
    cairo_fill(cr_);
}

} // namespace Component
