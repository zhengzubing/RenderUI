#include "Component/widgets/Button.hpp"
#include "Component/RenderContext.hpp"

namespace Component {

Button::Button(const std::string& text) : text_(text) {
    setSize(100, 40);  // 默认大小
}

Button::~Button() = default;

void Button::setText(const std::string& text) {
    if (text_ != text) {
        text_ = text;
        markDirty();
    }
}

void Button::setOnClick(ClickCallback callback) {
    onClick_ = std::move(callback);
}

void Button::setBackgroundColor(const Color& color) {
    bgColor_ = color;
    markDirty();
}

void Button::setTextColor(const Color& color) {
    textColor_ = color;
    markDirty();
}

void Button::setFontSize(float size) {
    fontSize_ = size;
    markDirty();
}

void Button::setCornerRadius(float radius) {
    cornerRadius_ = radius;
    markDirty();
}

void Button::onDraw(Canvas& canvas) {
    auto pos = getPosition();
    auto size = getSize();
    
    // 绘制背景（圆角矩形）
    Color bgColor = pressed_ ? 
        Color(bgColor_.r * 0.8f, bgColor_.g * 0.8f, bgColor_.b * 0.8f, bgColor_.a) : 
        bgColor_;
    
    canvas.drawRoundedRect(pos.x, pos.y, size.width, size.height, cornerRadius_, bgColor);
    
    // 绘制文本（居中）
    if (!text_.empty()) {
        // 简化文本居中计算
        float textWidth = text_.length() * fontSize_ * 0.6f;
        float textHeight = fontSize_ * 1.2f;
        float textX = pos.x + (size.width - textWidth) / 2;
        float textY = pos.y + (size.height + textHeight) / 2;
        
        canvas.drawText(textX, textY, text_.c_str(), "Sans", fontSize_, textColor_);
    }
}

bool Button::handleTouchEvent(const TouchEvent& event) {
    auto pos = getPosition();
    auto size = getSize();
    
    switch (event.type) {
        case EventType::TouchDown:
            // 检查是否在按钮内
            if (event.x >= pos.x && event.x <= pos.x + size.width &&
                event.y >= pos.y && event.y <= pos.y + size.height) {
                pressed_ = true;
                touchStartX_ = event.x;
                touchStartY_ = event.y;
                markDirty();
                return true;
            }
            break;
            
        case EventType::TouchUp:
            if (pressed_) {
                // 检查是否在按钮内释放
                bool inBounds = event.x >= pos.x && event.x <= pos.x + size.width &&
                               event.y >= pos.y && event.y <= pos.y + size.height;
                
                pressed_ = false;
                markDirty();
                
                if (inBounds && onClick_) {
                    onClick_();
                }
                return true;
            }
            break;
            
        case EventType::TouchCancel:
            if (pressed_) {
                pressed_ = false;
                markDirty();
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

} // namespace Component
