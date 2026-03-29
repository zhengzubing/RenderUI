#pragma once

#include "Widget.hpp"
#include <string>
#include <functional>

namespace Component {

/**
 * @brief 复选框控件
 * 
 * 支持选中/未选中状态，可切换
 */
class CheckBox : public Widget {
public:
    using ChangeCallback = std::function<void(bool checked)>;
    
    explicit CheckBox(const std::string& text = "");
    ~CheckBox() override;
    
    /**
     * @brief 设置文本标签
     */
    void setText(const std::string& text);
    
    /**
     * @brief 获取文本标签
     */
    const std::string& getText() const { return text_; }
    
    /**
     * @brief 设置选中状态
     */
    void setChecked(bool checked);
    
    /**
     * @brief 获取选中状态
     */
    bool isChecked() const { return checked_; }
    
    /**
     * @brief 切换选中状态
     */
    void toggle();
    
    /**
     * @brief 设置改变事件回调
     */
    void setOnChange(ChangeCallback callback);
    
    /**
     * @brief 设置选中时颜色
     */
    void setCheckedColor(const Color& color);
    
    /**
     * @brief 设置未选中时颜色
     */
    void setUncheckedColor(const Color& color);
    
protected:
    void onDraw(Canvas& canvas) override;
    bool handleTouchEvent(const TouchEvent& event) override;
    
private:
    std::string text_;
    bool checked_ = false;
    float boxSize_ = 20.0f;
    Color checkedColor_{0.2f, 0.6f, 0.9f, 1.0f};   // 蓝色
    Color uncheckedColor_{0.8f, 0.8f, 0.8f, 1.0f}; // 灰色
    
    ChangeCallback onChange_;
    bool pressed_ = false;
};

} // namespace Component
