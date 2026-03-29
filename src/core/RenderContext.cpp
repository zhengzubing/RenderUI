#include "RenderContext.hpp"
#include "Logger.hpp"
#include <cstring>
#include <cmath>

namespace Component {

struct RenderContext::Impl {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    cairo_t* cairo = nullptr;
    cairo_surface_t* cairoSurface = nullptr;
    bool initialized = false;
};

RenderContext::RenderContext() : impl_(std::make_unique<Impl>()) {}

RenderContext::~RenderContext() {
    cleanup();
}

bool RenderContext::init(EGLDisplay display, EGLSurface surface) {
    impl_->display = display;
    impl_->surface = surface;
    
    // 获取 EGL 表面大小
    int width = 0, height = 0;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    
    // 创建 Cairo 表面 (使用 OpenGL 纹理作为目标)
    // 注意：这里简化处理，实际需要使用 cairo_gl_surface_create
    impl_->cairoSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (!impl_->cairoSurface) {
        LOG_ERROR("Failed to create Cairo surface");
        return false;
    }
    
    impl_->cairo = cairo_create(impl_->cairoSurface);
    if (!impl_->cairo) {
        LOG_ERROR("Failed to create Cairo context");
        cairo_surface_destroy(impl_->cairoSurface);
        impl_->cairoSurface = nullptr;
        return false;
    }
    
    impl_->initialized = true;
    LOG_INFO("RenderContext initialized: %dx%d", width, height);
    return true;
}

cairo_t* RenderContext::getCairoContext() const {
    return impl_->cairo;
}

EGLSurface RenderContext::getEglSurface() const {
    return impl_->surface;
}

EGLDisplay RenderContext::getEglDisplay() const {
    return impl_->display;
}

void RenderContext::beginFrame() {
    if (!impl_->initialized || !impl_->cairo) {
        return;
    }
    
    // 清空画布
    cairo_save(impl_->cairo);
    cairo_set_source_rgba(impl_->cairo, 0, 0, 0, 0);
    cairo_paint(impl_->cairo);
    cairo_restore(impl_->cairo);
}

void RenderContext::endFrame() {
    if (!impl_->initialized) {
        return;
    }
    
    // 交换 EGL 缓冲区
    eglSwapBuffers(impl_->display, impl_->surface);
}

void RenderContext::cleanup() {
    if (impl_->cairo) {
        cairo_destroy(impl_->cairo);
        impl_->cairo = nullptr;
    }
    
    if (impl_->cairoSurface) {
        cairo_surface_destroy(impl_->cairoSurface);
        impl_->cairoSurface = nullptr;
    }
    
    impl_->initialized = false;
}

// ============================================================================
// Canvas 实现
// ============================================================================

Canvas::Canvas(cairo_t* cr) : cr_(cr) {}

void Canvas::setColor(const Color& color) {
    if (cr_) {
        cairo_set_source_rgba(cr_, color.r, color.g, color.b, color.a);
    }
}

void Canvas::drawRect(float x, float y, float width, float height, const Color& color) {
    if (!cr_) return;
    
    cairo_set_source_rgba(cr_, color.r, color.g, color.b, color.a);
    cairo_set_line_width(cr_, 1.0);
    cairo_rectangle(cr_, x, y, width, height);
    cairo_stroke(cr_);
}

void Canvas::fillRect(float x, float y, float width, float height, const Color& color) {
    if (!cr_) return;
    
    cairo_set_source_rgba(cr_, color.r, color.g, color.b, color.a);
    cairo_rectangle(cr_, x, y, width, height);
    cairo_fill(cr_);
}

void Canvas::drawText(float x, float y, const char* text, const char* font, float size, const Color& color) {
    if (!cr_ || !text) return;
    
    cairo_select_font_face(cr_, font ? font : "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr_, size);
    cairo_set_source_rgba(cr_, color.r, color.g, color.b, color.a);
    
    cairo_move_to(cr_, x, y);
    cairo_show_text(cr_, text);
}

void Canvas::drawImage(float x, float y, float width, float height, cairo_surface_t* surface) {
    if (!cr_ || !surface) return;
    
    cairo_save(cr_);
    cairo_set_source_surface(cr_, surface, x, y);
    cairo_pattern_set_filter(cairo_get_source(cr_), CAIRO_FILTER_BEST);
    cairo_rectangle(cr_, x, y, width, height);
    cairo_fill(cr_);
    cairo_restore(cr_);
}

void Canvas::drawRoundedRect(float x, float y, float width, float height, float radius, const Color& color) {
    if (!cr_) return;
    
    cairo_new_path(cr_);
    
    // 绘制圆角矩形路径
    cairo_arc(cr_, x + radius, y + radius, radius, M_PI, 3 * M_PI / 2);  // 左上
    cairo_arc(cr_, x + width - radius, y + radius, radius, 3 * M_PI / 2, 2 * M_PI);  // 右上
    cairo_arc(cr_, x + width - radius, y + height - radius, radius, 0, M_PI / 2);  // 右下
    cairo_arc(cr_, x + radius, y + height - radius, radius, M_PI / 2, M_PI);  // 左下
    cairo_close_path(cr_);
    
    cairo_set_source_rgba(cr_, color.r, color.g, color.b, color.a);
    cairo_fill(cr_);
}

} // namespace Component
