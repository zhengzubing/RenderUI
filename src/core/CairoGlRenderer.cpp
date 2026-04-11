#include "CairoGlRenderer.hpp"
#include "Logger.hpp"
#include <GLES2/gl2.h>
#include <cstring>
#include <cmath>

namespace Component {

CairoGlRenderer::CairoGlRenderer() = default;

CairoGlRenderer::~CairoGlRenderer() {
    cleanup();
}

bool CairoGlRenderer::init(EGLDisplay display, EGLSurface surface) {
    display_ = display;
    surface_ = surface;
    
    // 获取 EGL 表面大小
    int width = 0, height = 0;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);
    LOG_I << "EGL surface size: " << width << "x" << height;
    
    // 创建 Cairo 表面 (使用 OpenGL 纹理作为目标)
    // 注意：这里简化处理，实际需要使用 cairo_gl_surface_create
    cairoSurface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (!cairoSurface_) {
        LOG_E << "Failed to create Cairo surface";
        return false;
    }
    
    cairo_ = cairo_create(cairoSurface_);
    if (!cairo_) {
        LOG_E << "Failed to create Cairo context";
        cairo_surface_destroy(cairoSurface_);
        cairoSurface_ = nullptr;
        return false;
    }
    
    initialized_ = true;
    LOG_I << "CairoGlRenderer initialized: " << width << "x" << height;
    return true;
}

cairo_t* CairoGlRenderer::getCairoContext() const {
    return cairo_;
}

EGLSurface CairoGlRenderer::getEglSurface() const {
    return surface_;
}

EGLDisplay CairoGlRenderer::getEglDisplay() const {
    return display_;
}

void CairoGlRenderer::beginFrame() {
    if (!initialized_ || !cairo_) {
        return;
    }

    // TODO
}

void CairoGlRenderer::endFrame() {
    if (!initialized_) {
        return;
    }
    
    // ⚠️ 关键修复：将 Cairo image surface 的内容同步到 EGL surface
    int width = cairo_image_surface_get_width(cairoSurface_);
    int height = cairo_image_surface_get_height(cairoSurface_);
    unsigned char* data = cairo_image_surface_get_data(cairoSurface_);
    
    if (data) {
        // 确保 Cairo 绘制完成
        cairo_surface_flush(cairoSurface_);
        
        // 设置视口
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // 创建纹理
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Cairo 使用 ARGB32 格式，在 little-endian 系统上内存布局是 BGRA
        // OpenGL ES 不直接支持 GL_BGRA，所以我们保持 GL_RGBA 但需要调整
        // 或者直接使用 GL_RGBA 并接受颜色通道交换（临时方案）
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        
        // 简单的着色器程序（如果还没有创建）
        static GLuint program = 0;
        if (program == 0) {
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
            
            program = glCreateProgram();
            glAttachShader(program, vertShader);
            glAttachShader(program, fragShader);
            glLinkProgram(program);
            
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
        }
        
        glUseProgram(program);
        
        // 顶点数据 (x, y, u, v)
        float vertices[] = {
            -1.0f, -1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 1.0f,
             1.0f,  1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 0.0f
        };
        
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        GLint aPos = glGetAttribLocation(program, "aPos");
        GLint aTex = glGetAttribLocation(program, "aTex");
        
        glEnableVertexAttribArray(aPos);
        glVertexAttribPointer(aPos, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(aTex);
        glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &texture);        
    }
    
    LOG_I << "CairoGlRenderer endFrame: width" << width << " height" << height;
    // 交换 EGL 缓冲区
    eglSwapBuffers(display_, surface_);
}

void CairoGlRenderer::cleanup() {
    if (cairo_) {
        cairo_destroy(cairo_);
        cairo_ = nullptr;
    }
    
    if (cairoSurface_) {
        cairo_surface_destroy(cairoSurface_);
        cairoSurface_ = nullptr;
    }
    
    initialized_ = false;
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
    
    LOG_I << "Canvas drawRoundedRect" << x << "," << y << " " << width << "," << height << " " << radius;
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
