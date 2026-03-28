#pragma once

#include "Component/Widget.hpp"
#include <string>
#include <functional>

namespace Component {

/**
 * @brief 按钮控件
 * 
 * 支持点击事件和自定义回调
 */
class Button : public Widget {
public:
    using ClickCallback = std::function<void()>;
    
    explicit Button(const std::string& text = "");
    ~Button() override;
    
    /**
     * @brief 设置按钮文本
     */
    void setText(const std::string& text);
    
    /**
     * @brief 获取按钮文本
     */
    const std::string& getText() const { return text_; }
    
    /**
     * @brief 设置点击回调
     */
    void setOnClick(ClickCallback callback);
    
    /**
     * @brief 设置背景颜色
     */
    void setBackgroundColor(const Color& color);
    
    /**
     * @brief 获取背景颜色
     */
    Color getBackgroundColor() const { return bgColor_; }
    
    /**
     * @brief 设置文本颜色
     */
    void setTextColor(const Color& color);
    
    /**
     * @brief 获取文本颜色
     */
    Color getTextColor() const { return textColor_; }
    
    /**
     * @brief 设置字体大小
     */
    void setFontSize(float size);
    
    /**
     * @brief 设置圆角半径
     */
    void setCornerRadius(float radius);
    
    /**
     * @brief 是否被按下
     */
    bool isPressed() const { return pressed_; }
    
protected:
    void onDraw(Canvas& canvas) override;
    bool handleTouchEvent(const TouchEvent& event) override;
    
private:
    std::string text_;
    Color bgColor_{0.2f, 0.2f, 0.8f, 1.0f};  // 默认蓝色
    Color textColor_{1.0f, 1.0f, 1.0f, 1.0f};  // 白色
    float fontSize_ = 16.0f;
    float cornerRadius_ = 4.0f;
    
    bool pressed_ = false;
    ClickCallback onClick_;
    
    float touchStartX_ = 0;
    float touchStartY_ = 0;
};

} // namespace Component
