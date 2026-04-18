#include "widgets/CheckBox.hpp"
#include "CairoGlRenderer.hpp"
#include "EventLoop.hpp"

namespace Component {

CheckBox::CheckBox(const std::string& text) : text_(text) {
    setSize(100, 30);  // 默认大小
}

CheckBox::~CheckBox() = default;

void CheckBox::setText(const std::string& text) {
    if (text_ != text) {
        text_ = text;
        markDirty();
    }
}

void CheckBox::setChecked(bool checked) {
    if (checked_ != checked) {
        checked_ = checked;
        if (onChange_) {
            onChange_(checked_);
        }
        markDirty();
    }
}

void CheckBox::toggle() {
    setChecked(!checked_);
}

void CheckBox::setOnChange(ChangeCallback callback) {
    onChange_ = std::move(callback);
}

void CheckBox::setCheckedColor(const Color& color) {
    checkedColor_ = color;
    markDirty();
}

void CheckBox::setUncheckedColor(const Color& color) {
    uncheckedColor_ = color;
    markDirty();
}

void CheckBox::onDraw(Canvas& canvas) {
    auto pos = getPosition();
    auto size = getSize();
    
    // 绘制复选框
    float boxX = pos.x + 5;
    float boxY = pos.y + (size.height - boxSize_) / 2;
    
    Color boxColor = checked_ ? checkedColor_ : uncheckedColor_;
    canvas.fillRect(boxX, boxY, boxSize_, boxSize_, boxColor);
    
    // 如果选中，绘制对勾
    if (checked_) {
        // 简化绘制对勾（使用两条线）
        canvas.setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));  // 白色对勾
        // TODO: 使用 canvas.drawLine 绘制对勾
    }
    
    // 绘制文本标签
    if (!text_.empty()) {
        float textX = boxX + boxSize_ + 8;
        float textY = pos.y + size.height / 2 + 6;
        canvas.drawText(textX, textY, text_.c_str(), "Sans", 16.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    }
}

bool CheckBox::handleTouchEvent(const TouchEvent& event) {
    auto pos = getPosition();
    auto size = getSize();
    
    switch (event.type) {
        case EventType::TouchDown:
            if (event.x >= pos.x && event.x <= pos.x + size.width &&
                event.y >= pos.y && event.y <= pos.y + size.height) {
                pressed_ = true;
                markDirty();
                return true;
            }
            break;
            
        case EventType::TouchUp:
            if (pressed_) {
                bool inBounds = event.x >= pos.x && event.x <= pos.x + size.width &&
                               event.y >= pos.y && event.y <= pos.y + size.height;
                
                pressed_ = false;
                
                if (inBounds) {
                    toggle();
                }
                
                markDirty();
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
