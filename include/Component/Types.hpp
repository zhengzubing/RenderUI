#pragma once

#include <cstdint>

namespace Component {

/**
 * @brief 二维向量
 */
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
    
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
};

/**
 * @brief 尺寸
 */
struct Size {
    float width = 0.0f;
    float height = 0.0f;
    
    Size() = default;
    Size(float w, float h) : width(w), height(h) {}
};

/**
 * @brief 矩形区域
 */
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    
    Rect() = default;
    Rect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
};

/**
 * @brief 颜色 (RGBA)
 */
struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
    
    Color() = default;
    Color(float r, float g, float b, float a = 1.0f) 
        : r(r), g(g), b(b), a(a) {}
    
    // 从十六进制字符串创建，如 "#FFFFFF"
    static Color fromHex(const char* hex) {
        Color c;
        c.r = c.g = c.b = 0.0f;
        c.a = 1.0f;
        
        if (hex[0] == '#') {
            hex++;
        }
        
        unsigned int val = 0;
        if (sscanf(hex, "%6x", &val) == 1) {
            c.r = ((val >> 16) & 0xFF) / 255.0f;
            c.g = ((val >> 8) & 0xFF) / 255.0f;
            c.b = (val & 0xFF) / 255.0f;
        }
        
        return c;
    }
};

} // namespace Component
