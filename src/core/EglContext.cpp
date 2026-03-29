#include "EglContext.hpp"
#include "Logger.hpp"

namespace Component {

EglContext::EglContext() = default;

EglContext::~EglContext() {
    cleanup();
}

bool EglContext::init(void* display) {
    if (initialized_) {
        LOG_WARNING << "EGL already initialized";
        return true;
    }
    
    // 获取 EGL 显示连接
    display_ = eglGetDisplay((EGLNativeDisplayType)display);
    if (display_ == EGL_NO_DISPLAY) {
        LOG_ERROR << "Failed to get EGL display";
        return false;
    }
    
    // 初始化 EGL
    if (!eglInitialize(display_, nullptr, nullptr)) {
        LOG_ERROR << "Failed to initialize EGL";
        display_ = EGL_NO_DISPLAY;
        return false;
    }
    
    // 配置 EGL
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };
    
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display_, configAttribs, &config_, 1, &numConfigs)) {
        LOG_ERROR << "Failed to choose EGL config";
        cleanup();
        return false;
    }
    
    if (numConfigs == 0) {
        LOG_ERROR << "No suitable EGL config found";
        cleanup();
        return false;
    }
    
    // 创建 EGL 上下文
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    context_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, contextAttribs);
    if (context_ == EGL_NO_CONTEXT) {
        LOG_ERROR << "Failed to create EGL context";
        cleanup();
        return false;
    }
    
    initialized_ = true;
    LOG_INFO << "EGL initialized successfully";
    return true;
}

EGLSurface EglContext::createSurface(void* nativeWindow, int width, int height) {
    if (!initialized_) {
        LOG_ERROR << "EGL not initialized";
        return EGL_NO_SURFACE;
    }
    
    // 创建 EGL 表面
    const EGLint surfaceAttribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };
    
    EGLSurface surface = eglCreateWindowSurface(
        display_, config_, (EGLNativeWindowType)nativeWindow, surfaceAttribs);
    
    if (surface == EGL_NO_SURFACE) {
        LOG_ERROR << "Failed to create EGL surface";
        return EGL_NO_SURFACE;
    }
    
    LOG_INFO << "EGL surface created: " << width << "x" << height;
    return surface;
}

bool EglContext::makeCurrent(EGLSurface surface) {
    if (!initialized_) {
        return false;
    }
    
    if (!eglMakeCurrent(display_, surface, surface, context_)) {
        LOG_ERROR << "Failed to make EGL context current";
        return false;
    }
    
    return true;
}

void EglContext::swapBuffers(EGLSurface surface) {
    if (initialized_ && surface != EGL_NO_SURFACE) {
        eglSwapBuffers(display_, surface);
    }
}

void EglContext::cleanup() {
    if (context_ != EGL_NO_CONTEXT) {
        eglDestroyContext(display_, context_);
        context_ = EGL_NO_CONTEXT;
    }
    
    if (display_ != EGL_NO_DISPLAY) {
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
    
    initialized_ = false;
    LOG_INFO << "EGL cleaned up";
}

} // namespace Component
