#include "SkiaRenderer.hpp"
#include "Logger.hpp"

#ifdef RENDERUI_USE_SKIA

#include <SkSurface.h>
#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkFont.h>
#include <SkTypeface.h>
#include <SkColor.h>
#include <GrDirectContext.h>
#include <GL/gl.h>

namespace Component {

struct SkiaRenderer::Impl {
    std::unique_ptr<SkSurface> surface;
    sk_sp<GrDirectContext> grContext;
    GLuint framebuffer = 0;
    GLuint texture = 0;
};

struct SkiaPaint::Impl {
    std::unique_ptr<SkPaint> paint;
    
    Impl() : paint(std::make_unique<SkPaint>()) {}
};

struct SkiaFont::Impl {
    std::unique_ptr<SkFont> font;
    std::string fontFamily;
    float size;
    
    Impl(const std::string& family, float s) 
        : font(std::make_unique<SkFont>()), fontFamily(family), size(s) {
        auto typeface = SkTypeface::MakeFromName(family.c_str(), SkFontStyle::Normal());
        font->setTypeface(typeface);
        font->setSize(s);
    }
};

// ============================================================================
// SkiaRenderer
// ============================================================================

SkiaRenderer::SkiaRenderer() : impl_(std::make_unique<Impl>()) {}

SkiaRenderer::~SkiaRenderer() {
    cleanup();
}

bool SkiaRenderer::init(int width, int height, bool useGpu) {
    if (initialized_) {
        return true;
    }
    
    width_ = width;
    height_ = height;
    useGpu_ = useGpu;
    
    if (useGpu) {
        // GPU 加速模式
        // 创建 OpenGL 纹理
        glGenTextures(1, &impl_->texture);
        glBindTexture(GL_TEXTURE_2D, impl_->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        
        // 创建 FBO
        glGenFramebuffers(1, &impl_->framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, impl_->framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                               GL_TEXTURE_2D, impl_->texture, 0);
        
        // 创建 GrDirectContext
        impl_->grContext = GrDirectContext::MakeGL(glGetIntegerv);
        
        if (!impl_->grContext) {
            LOG_ERROR << "Failed to create GrDirectContext";
            return false;
        }
        
        // 创建 SkSurface
        GrGLFramebufferInfo fbInfo;
        fbInfo.fFBOID = impl_->framebuffer;
        fbInfo.fFormat = GL_RGBA8;
        
        GrBackendRenderTarget backendRT(width, height, 0, 8, fbInfo);
        
        SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag);
        impl_->surface = SkSurfaces::WrapBackendRenderTarget(
            impl_->grContext.get(), backendRT, kBottomLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType, nullptr, &props);
        
        if (!impl_->surface) {
            LOG_ERROR << "Failed to create SkSurface";
            return false;
        }
        
        LOG_INFO << "SkiaRenderer initialized with GPU: " << width << "x" << height;
    } else {
        // CPU 模式（简化实现，使用 raster 表面）
        // TODO: 实现 CPU 模式
        LOG_WARNING << "CPU mode not fully implemented yet";
    }
    
    initialized_ = true;
    return true;
}

SkCanvas* SkiaRenderer::getCanvas() const {
    if (impl_->surface) {
        return impl_->surface->getCanvas();
    }
    return nullptr;
}

SkSurface* SkiaRenderer::getSurface() const {
    return impl_->surface.get();
}

void SkiaRenderer::beginFrame() {
    if (!impl_->surface) {
        return;
    }
    
    auto canvas = impl_->surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
}

void SkiaRenderer::endFrame() {
    if (!impl_->surface) {
        return;
    }
    
    impl_->surface->flushAndSubmit();
    
    if (useGpu_) {
        // 将纹理内容渲染到屏幕
        // TODO: 实现最终的 blit 操作
    }
}

void SkiaRenderer::resize(int width, int height) {
    if (width == width_ && height == height_) {
        return;
    }
    
    cleanup();
    init(width, height, useGpu_);
}

void SkiaRenderer::cleanup() {
    if (impl_->framebuffer != 0) {
        glDeleteFramebuffers(1, &impl_->framebuffer);
        impl_->framebuffer = 0;
    }
    
    if (impl_->texture != 0) {
        glDeleteTextures(1, &impl_->texture);
        impl_->texture = 0;
    }
    
    impl_->surface.reset();
    impl_->grContext.reset();
    initialized_ = false;
}

// ============================================================================
// SkiaPaint
// ============================================================================

SkiaPaint::SkiaPaint() : impl_(std::make_unique<Impl>()) {}

SkiaPaint::~SkiaPaint() = default;

void SkiaPaint::setColor(const Color& color) {
    impl_->paint->setColor(SkColorSetARGB(
        static_cast<U8CPU>(color.a * 255),
        static_cast<U8CPU>(color.r * 255),
        static_cast<U8CPU>(color.g * 255),
        static_cast<U8CPU>(color.b * 255)
    ));
}

void SkiaPaint::setStrokeWidth(float width) {
    impl_->paint->setStrokeWidth(width);
}

void SkiaPaint::setAntiAlias(bool aa) {
    impl_->paint->setAntiAlias(aa);
}

void SkiaPaint::setFill() {
    impl_->paint->setStyle(SkPaint::kFill_Style);
}

void SkiaPaint::setStroke() {
    impl_->paint->setStyle(SkPaint::kStroke_Style);
}

SkPaint* SkiaPaint::getSkPaint() const {
    return impl_->paint.get();
}

// ============================================================================
// SkiaFont
// ============================================================================

SkiaFont::SkiaFont(const std::string& fontFamily, float size) 
    : impl_(std::make_unique<Impl>(fontFamily, size)) {}

SkiaFont::~SkiaFont() = default;

void SkiaFont::setFontFamily(const std::string& family) {
    impl_->fontFamily = family;
    auto typeface = SkTypeface::MakeFromName(family.c_str(), SkFontStyle::Normal());
    impl_->font->setTypeface(typeface);
}

void SkiaFont::setSize(float size) {
    impl_->size = size;
    impl_->font->setSize(size);
}

SkFont* SkiaFont::getSkFont() const {
    return impl_->font.get();
}

} // namespace Component

#else

// 空实现（当未启用 Skia 时）
namespace Component {

struct SkiaRenderer::Impl {};
struct SkiaPaint::Impl {};
struct SkiaFont::Impl {};

SkiaRenderer::SkiaRenderer() : impl_(std::make_unique<Impl>()) {}
SkiaRenderer::~SkiaRenderer() = default;

bool SkiaRenderer::init(int width, int height, bool useGpu) {
    LOG_WARNING << "Skia support not enabled. Recompile with -DUSE_SKIA=ON";
    return false;
}

SkCanvas* SkiaRenderer::getCanvas() const { return nullptr; }
SkSurface* SkiaRenderer::getSurface() const { return nullptr; }
void SkiaRenderer::beginFrame() {}
void SkiaRenderer::endFrame() {}
void SkiaRenderer::resize(int w, int h) {}
void SkiaRenderer::cleanup() {}

SkiaPaint::SkiaPaint() : impl_(std::make_unique<Impl>()) {}
SkiaPaint::~SkiaPaint() = default;
void SkiaPaint::setColor(const Color&) {}
void SkiaPaint::setStrokeWidth(float) {}
void SkiaPaint::setAntiAlias(bool) {}
void SkiaPaint::setFill() {}
void SkiaPaint::setStroke() {}
SkPaint* SkiaPaint::getSkPaint() const { return nullptr; }

SkiaFont::SkiaFont(const std::string&, float) : impl_(std::make_unique<Impl>()) {}
SkiaFont::~SkiaFont() = default;
void SkiaFont::setFontFamily(const std::string&) {}
void SkiaFont::setSize(float) {}
SkFont* SkiaFont::getSkFont() const { return nullptr; }

} // namespace Component

#endif
